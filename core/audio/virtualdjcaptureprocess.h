#ifndef PHOTON_VIRTUALDJCAPTUREPROCESS_H
#define PHOTON_VIRTUALDJCAPTUREPROCESS_H

#include <QVector>
#include <QString>
#include "audioprocessor.h"

namespace photon {

/**
 * Records a live beat grid from VirtualDJ instead of decoding a local file - the
 * producer for tracks with no accessible audio file (streaming sources played
 * through VDJ). Tells the plugin to restart and play the active deck, listens to
 * VirtualDJConnector's telemetry as it plays, and finishes (auto at song end, or on
 * stopCapture()) by writing a BeatGrid into the sequence's SongData.
 *
 * Beat timestamps are back-calculated from VDJ's fractional beat position and bpm
 * (time - beatProgress * 60/bpm) rather than stamped at whatever tick happens to
 * observe the beat number incrementing, so precision doesn't depend on the
 * connector's tick rate - see the "Beat-capture precision" note in the VDJ capture
 * plan. The plugin is still asked to tick faster (sendCaptureStart/Stop) purely to
 * tighten end-of-song detection and reduce the chance of missing a beat outright at
 * fast tempos.
 *
 * Level and stem levels (Vocal/Instru/Bass/Kick/HiHat - VDJ's own real-time AI stem
 * separation) come straight from VDJ's engine (get_level) rather than raw audio
 * sampling - one sample per tick, appended unconditionally (unlike beats, these are
 * continuous, not discrete events). The resulting FeatureTrack sample rate is
 * derived from the actual achieved tick rate over the capture (sample count /
 * elapsed time) rather than assumed from the requested capture-mode interval, since
 * ticks may not land exactly on schedule.
 */
class PHOTONCORE_EXPORT VirtualDJCaptureProcess : public AudioProcessor
{
    Q_OBJECT
public:
    VirtualDJCaptureProcess();

    void startProcessing() override;
    void processingComplete() override;

    // Manual early-stop; shares the same finalize path as automatic (song-end) stop.
    void stopCapture();

    static AudioProcessorInformation info();

private slots:
    void onDataUpdated();

private:
    void finalize();

    bool m_active = false;
    int m_lastBeatNumber = -1;
    QVector<double> m_beats;

    // Level/stem samples, one per tick, alongside each one's real capture time
    // (m_sampleTimes) - VDJ's ticks don't land at perfectly even intervals, so
    // this is what lets processingComplete() resample onto FeatureTrack's
    // required uniform grid accurately instead of just assuming the raw samples
    // already are evenly spaced. m_firstSampleTime/m_lastSampleTime (song time,
    // from the connector) bound the capture so the achieved sample rate can be
    // derived at the end.
    QVector<double> m_sampleTimes;
    QVector<float> m_levelSamples;
    QVector<float> m_vocalSamples;
    QVector<float> m_instruSamples;
    QVector<float> m_bassSamples;
    QVector<float> m_kickSamples;
    QVector<float> m_hiHatSamples;
    double m_firstSampleTime = 0.0;
    double m_lastSampleTime = 0.0;

    // Snapshot of the track being captured, taken at start - used to detect the user
    // changing tracks in VDJ mid-capture, and to populate SongData at the end.
    QString m_expectedPath;
    QString m_expectedTitle;
    QString m_expectedArtist;
    double m_expectedSongLength = 0.0;
};

} // namespace photon

#endif // PHOTON_VIRTUALDJCAPTUREPROCESS_H
