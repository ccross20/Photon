#include <algorithm>
#include <cmath>
#include <QDebug>
#include "virtualdjcaptureprocess.h"
#include "songdata.h"
#include "sequence/sequence.h"
#include "virtualdj/virtualdjconnector.h"
#include "photoncore.h"

namespace photon {

VirtualDJCaptureProcess::VirtualDJCaptureProcess() : AudioProcessor()
{
}

void VirtualDJCaptureProcess::startProcessing()
{
    VirtualDJConnector *connector = photonApp->djConnector();

    if(!connector->isConnected())
    {
        qWarning() << "VirtualDJCaptureProcess: not connected to VirtualDJ, aborting capture.";
        complete();
        return;
    }

    // Snapshot the track being captured, used both to detect the user switching
    // tracks in VDJ mid-capture and to populate SongData's metadata at the end.
    m_expectedPath = connector->path;
    m_expectedTitle = connector->title;
    m_expectedArtist = connector->artist;
    m_expectedSongLength = connector->songLength;

    m_beats.clear();
    m_lastBeatNumber = -1;
    m_levelSamples.clear();
    m_vocalSamples.clear();
    m_instruSamples.clear();
    m_bassSamples.clear();
    m_kickSamples.clear();
    m_hiHatSamples.clear();
    m_firstSampleTime = 0.0;
    m_lastSampleTime = 0.0;
    m_active = true;

    connect(connector, &VirtualDJConnector::dataUpdated, this, &VirtualDJCaptureProcess::onDataUpdated);

    // Arm the plugin's faster tick rate before playback starts, so the earliest
    // beats are captured at the same precision as the rest of the track.
    connector->sendCaptureStart();
    connector->sendRestart();
    connector->sendPlay();
}

void VirtualDJCaptureProcess::onDataUpdated()
{
    if(!m_active)
        return;

    VirtualDJConnector *connector = photonApp->djConnector();

    // Level/stems are continuous, unlike beats - one sample per tick,
    // unconditionally, straight from VDJ's own reported values.
    if(m_levelSamples.isEmpty())
        m_firstSampleTime = connector->time;
    m_lastSampleTime = connector->time;
    m_levelSamples.append(static_cast<float>(connector->level));
    m_vocalSamples.append(static_cast<float>(connector->stemVocal));
    m_instruSamples.append(static_cast<float>(connector->stemInstru));
    m_bassSamples.append(static_cast<float>(connector->stemBass));
    m_kickSamples.append(static_cast<float>(connector->stemKick));
    m_hiHatSamples.append(static_cast<float>(connector->stemHiHat));

    // Restrict to a plausible DJ-track bpm range: a transient glitch reading near
    // zero (e.g. in the brief window right after play starts, before VDJ's bpm
    // detection has stabilised) would otherwise make 60/bpm huge and produce a wildly
    // wrong back-calculated time - which downstream rendering has to trust blindly.
    if(connector->beatNumber != m_lastBeatNumber && connector->beatNumber >= 0
       && connector->bpm >= 20.0 && connector->bpm <= 999.0)
    {
        // Back-calculate the exact beat-crossing time from VDJ's fractional beat
        // position and bpm, rather than stamping whichever tick happened to observe
        // the beat number incrementing - keeps precision independent of tick rate.
        const double exactBeatTime = connector->time - connector->beatProgress * (60.0 / connector->bpm);

        // A crossing must be finite and at-or-before now; reject and retry on the
        // next tick (m_lastBeatNumber is only advanced once a beat is actually
        // accepted) rather than silently banking a bad value.
        if(std::isfinite(exactBeatTime) && exactBeatTime >= 0.0 && exactBeatTime <= connector->time)
        {
            m_beats.append(exactBeatTime);
            m_lastBeatNumber = connector->beatNumber;
        }
    }

    const bool songEnded = m_expectedSongLength > 0.0 && connector->time >= (m_expectedSongLength - 0.5);
    const bool trackChanged = connector->title != m_expectedTitle || connector->artist != m_expectedArtist;

    if(songEnded || trackChanged)
        finalize();
}

void VirtualDJCaptureProcess::stopCapture()
{
    // Manual cancel stops VDJ playback too - unlike the natural song-end path (which
    // leaves VDJ alone, e.g. mid-automix), an early cancel means the user is
    // deliberately abandoning this capture.
    if(m_active)
        photonApp->djConnector()->sendPause();

    finalize();
}

void VirtualDJCaptureProcess::finalize()
{
    if(!m_active)
        return;
    m_active = false;

    VirtualDJConnector *connector = photonApp->djConnector();
    disconnect(connector, &VirtualDJConnector::dataUpdated, this, &VirtualDJCaptureProcess::onDataUpdated);
    connector->sendCaptureStop();

    complete();
}

void VirtualDJCaptureProcess::processingComplete()
{
    SongData *songData = sequence()->songData();

    songData->beats().setBeats(m_beats);
    songData->setSource("virtualdj-live");
    songData->setSourcePath(QString());   // no local file - this is the streaming-track case
    songData->setTitle(m_expectedTitle);
    songData->setArtist(m_expectedArtist);
    songData->setDuration(m_expectedSongLength);

    // Estimate bpm from the median inter-beat interval - robust to a single missed
    // or doubled beat, which would skew a plain average.
    if(m_beats.size() > 1)
    {
        QVector<double> intervals;
        intervals.reserve(m_beats.size() - 1);
        for(int i = 1; i < m_beats.size(); ++i)
            intervals.append(m_beats[i] - m_beats[i - 1]);
        std::sort(intervals.begin(), intervals.end());

        const double median = intervals[intervals.size() / 2];
        if(median > 0.0)
            songData->beats().setBpm(60.0 / median);
    }

    // Feature sample rate is derived from what was actually achieved (sample count
    // over elapsed capture time), not assumed from the requested capture-mode tick
    // interval - ticks may not have landed exactly on schedule.
    const double elapsed = m_lastSampleTime - m_firstSampleTime;
    const double featureRate = (m_levelSamples.size() > 1 && elapsed > 0.0)
                                    ? (m_levelSamples.size() - 1) / elapsed
                                    : 0.0;

    if(featureRate > 0.0)
    {
        // startTime accounts for the gap between sendRestart() and the first tick
        // actually landing (command latency) - samples don't necessarily begin at
        // song time exactly 0.
        auto addTrack = [&](const QByteArray &id, const QVector<float> &samples) {
            auto &track = songData->addFeature(id, featureRate);
            track.setStartTime(m_firstSampleTime);
            track.setSamples(samples);
        };

        addTrack(SongData::FeatureLevel, m_levelSamples);
        addTrack(SongData::FeatureVocal, m_vocalSamples);
        addTrack(SongData::FeatureInstru, m_instruSamples);
        addTrack(SongData::FeatureBass, m_bassSamples);
        addTrack(SongData::FeatureKick, m_kickSamples);
        addTrack(SongData::FeatureHiHat, m_hiHatSamples);
    }

    qDebug() << "VirtualDJCaptureProcess: captured" << m_beats.size() << "beats and"
             << m_levelSamples.size() << "level/stem samples ("
             << featureRate << "Hz achieved) over" << m_expectedSongLength << "seconds.";
}

AudioProcessorInformation VirtualDJCaptureProcess::info()
{
    AudioProcessorInformation toReturn([](){return new VirtualDJCaptureProcess;});
    toReturn.name = "Capture from VirtualDJ";
    toReturn.id = "photon.audio-process.virtualdj-capture";

    return toReturn;
}

} // namespace photon
