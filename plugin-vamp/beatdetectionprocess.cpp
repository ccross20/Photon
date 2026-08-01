#include <algorithm>
#include <QHash>
#include "beatdetectionprocess.h"
#include "sequence/sequence.h"
#include "audio/songdata.h"

namespace photon {

BeatDetectionProcess::BeatDetectionProcess()
{
    // qm-barbeattracker (Queen Mary QM Vamp Plugins) is a proper beat/bar tracker,
    // not an onset detector - it locks onto the metrical grid rather than firing on
    // every percussive transient, and labels each beat with its 1-based position in
    // the bar so we can recover true downbeats instead of guessing from beat 0.
    m_vamp.setPluginName("qm-vamp-plugins");
    m_vamp.setPluginId("qm-barbeattracker");

    connect(&m_vamp, &Vamp::processingComplete, this, &BeatDetectionProcess::complete);
}

void BeatDetectionProcess::startProcessing()
{

    m_vamp.setFile(audioFilePath());
    m_vamp.start();
}

void BeatDetectionProcess::processingComplete()
{
    const QList<BeatEvent> &events = m_vamp.beats();

    // SongData is the sole source of truth for detected beats - a companion .song
    // sidecar persists it, effects/graph nodes query it, and the waveform view
    // draws it directly. Beats are not a BeatLayer: layers are reserved for
    // user-authored custom cues, whereas the beat grid is derived data that's
    // simply always present once a file has been analysed.
    SongData *songData = sequence()->songData();

    QVector<double> beatSeconds;
    beatSeconds.reserve(events.size());
    for(const BeatEvent &e : events)
        beatSeconds.append(e.time);

    songData->beats().setBeats(beatSeconds);
    songData->setSource("vamp");
    songData->setSourcePath(sequence()->filePath());

    // Estimate bpm from the median inter-beat interval - robust to a single missed
    // or doubled beat, which would skew a plain average.
    if(beatSeconds.size() > 1)
    {
        QVector<double> intervals;
        intervals.reserve(beatSeconds.size() - 1);
        for(int i = 1; i < beatSeconds.size(); ++i)
            intervals.append(beatSeconds[i] - beatSeconds[i - 1]);
        std::sort(intervals.begin(), intervals.end());

        const double median = intervals[intervals.size() / 2];
        if(median > 0.0)
            songData->beats().setBpm(60.0 / median);
    }

    // qm-barbeattracker labels each beat with its 1-based position in the bar, so
    // the true bar length and beats[0]'s offset from the downbeat are both known
    // directly - no need to assume beats[0] is a downbeat.
    //
    // beatsPerBar can't be read off the label values directly: labels 1..N each
    // occur about once per bar, so they're all roughly equally frequent and the
    // mode of the raw label tells us nothing about N. What we actually want is the
    // mode of the BAR LENGTH - the beat count between consecutive downbeats -
    // which is robust to an occasional mistracked bar.
    QHash<int, int> barLengthCounts;
    int currentBarLength = 0;
    bool inBar = false;
    for(const BeatEvent &e : events)
    {
        if(e.barPosition <= 0)
            continue;
        if(e.barPosition == 1)
        {
            if(inBar && currentBarLength > 0)
                barLengthCounts[currentBarLength]++;
            currentBarLength = 1;
            inBar = true;
        }
        else if(inBar)
        {
            ++currentBarLength;
        }
    }
    // The final in-progress bar is deliberately not counted - it may be cut short
    // at end of file and would skew toward a too-small length.

    if(!barLengthCounts.isEmpty())
    {
        int beatsPerBar = 4;
        int bestCount = -1;
        for(auto it = barLengthCounts.constBegin(); it != barLengthCounts.constEnd(); ++it)
        {
            if(it.value() > bestCount)
            {
                bestCount = it.value();
                beatsPerBar = it.key();
            }
        }
        songData->beats().setBeatsPerBar(beatsPerBar);

        const int firstLabel = events.first().barPosition;
        if(firstLabel > 0)
            songData->beats().setBarStartOffset((firstLabel - 1) % beatsPerBar);
    }
}

AudioProcessorInformation BeatDetectionProcess::info()
{
    AudioProcessorInformation toReturn([](){return new BeatDetectionProcess;});
    toReturn.name = "Beats";
    toReturn.id = "photon.audio-process.beat";

    return toReturn;
}

} // namespace photon
