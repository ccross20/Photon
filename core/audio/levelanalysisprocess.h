#ifndef PHOTON_LEVELANALYSISPROCESS_H
#define PHOTON_LEVELANALYSISPROCESS_H

#include <QVector>
#include <QAudioDecoder>
#include "audioprocessor.h"

namespace photon {

/**
 * Decodes the audio file directly (no Vamp plugin involved) and derives an
 * overall level envelope (RMS) plus three frequency-band envelopes - low/mid/high
 * (peak) - stored as FeatureTracks on the sequence's SongData. Band splitting is a
 * lightweight three-filter split (RBJ-cookbook biquads), not a full spectrum
 * analyser - good enough to drive "pulse to the bass" / "sparkle on the hats"
 * style lighting effects, not audio-engineering-grade analysis.
 *
 * The low/mid/high bands track PEAK (not RMS) at a finer hop rate than level, so
 * that a waveform-style viewer drawing their per-pixel min/max (see
 * FeatureTrack::rangeBetween) shows real transient detail - a band-limited
 * waveform, not a smoothed energy blob.
 */
class PHOTONCORE_EXPORT LevelAnalysisProcess : public AudioProcessor
{
    Q_OBJECT
public:
    LevelAnalysisProcess();

    void startProcessing() override;
    void processingComplete() override;

    static AudioProcessorInformation info();

private slots:
    void bufferReady();
    void decodeFinished();

private:
    // Standard RBJ Audio EQ Cookbook biquad (Direct Form I transposed).
    struct Biquad
    {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
        double z1 = 0.0, z2 = 0.0;

        void setLowpass(double sampleRate, double cutoff, double q);
        void setHighpass(double sampleRate, double cutoff, double q);
        void setBandpass(double sampleRate, double centre, double q);
        float process(float x);
    };

    void processSample(float mono);
    void flushHop();

    QAudioDecoder *m_decoder;
    Biquad m_lowFilter;
    Biquad m_midFilter;
    Biquad m_highFilter;

    int m_sampleRate = 0;
    int m_channelCount = 0;
    int m_hopSize = 0;    // samples per output feature sample (~300 Hz)
    int m_hopCount = 0;   // samples accumulated into the current hop
    bool m_done = false;

    double m_levelSumSq = 0.0;   // RMS accumulator for the overall level
    float m_lowPeak = 0.0f;      // max |x| this hop, per band
    float m_midPeak = 0.0f;
    float m_highPeak = 0.0f;

    QVector<float> m_levelSamples;
    QVector<float> m_lowSamples;
    QVector<float> m_midSamples;
    QVector<float> m_highSamples;
};

} // namespace photon

#endif // PHOTON_LEVELANALYSISPROCESS_H
