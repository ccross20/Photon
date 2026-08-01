#include <cmath>
#include <algorithm>
#include <QDebug>
#include "levelanalysisprocess.h"
#include "songdata.h"
#include "sequence/sequence.h"

namespace photon {

// ---------------------------------------------------------------------------
// Biquad - RBJ Audio EQ Cookbook coefficient formulas, Direct Form I transposed
// processing. https://www.w3.org/andrew/edu/audio-eq-cookbook.html
// ---------------------------------------------------------------------------

void LevelAnalysisProcess::Biquad::setLowpass(double t_sampleRate, double t_cutoff, double t_q)
{
    t_cutoff = std::min(t_cutoff, t_sampleRate * 0.49);   // stay clear of Nyquist
    const double w0 = 2.0 * M_PI * t_cutoff / t_sampleRate;
    const double cosw0 = std::cos(w0);
    const double alpha = std::sin(w0) / (2.0 * t_q);
    const double a0 = 1.0 + alpha;

    b0 = ((1.0 - cosw0) / 2.0) / a0;
    b1 = (1.0 - cosw0) / a0;
    b2 = b0;
    a1 = (-2.0 * cosw0) / a0;
    a2 = (1.0 - alpha) / a0;
    z1 = z2 = 0.0;
}

void LevelAnalysisProcess::Biquad::setHighpass(double t_sampleRate, double t_cutoff, double t_q)
{
    t_cutoff = std::min(t_cutoff, t_sampleRate * 0.49);
    const double w0 = 2.0 * M_PI * t_cutoff / t_sampleRate;
    const double cosw0 = std::cos(w0);
    const double alpha = std::sin(w0) / (2.0 * t_q);
    const double a0 = 1.0 + alpha;

    b0 = ((1.0 + cosw0) / 2.0) / a0;
    b1 = (-(1.0 + cosw0)) / a0;
    b2 = b0;
    a1 = (-2.0 * cosw0) / a0;
    a2 = (1.0 - alpha) / a0;
    z1 = z2 = 0.0;
}

void LevelAnalysisProcess::Biquad::setBandpass(double t_sampleRate, double t_centre, double t_q)
{
    t_centre = std::min(t_centre, t_sampleRate * 0.49);
    const double w0 = 2.0 * M_PI * t_centre / t_sampleRate;
    const double cosw0 = std::cos(w0);
    const double alpha = std::sin(w0) / (2.0 * t_q);
    const double a0 = 1.0 + alpha;

    // Constant 0dB peak-gain bandpass.
    b0 = alpha / a0;
    b1 = 0.0;
    b2 = -alpha / a0;
    a1 = (-2.0 * cosw0) / a0;
    a2 = (1.0 - alpha) / a0;
    z1 = z2 = 0.0;
}

float LevelAnalysisProcess::Biquad::process(float t_x)
{
    const double y = b0 * t_x + z1;
    z1 = b1 * t_x - a1 * y + z2;
    z2 = b2 * t_x - a2 * y;
    return static_cast<float>(y);
}


// ---------------------------------------------------------------------------
// LevelAnalysisProcess
// ---------------------------------------------------------------------------

LevelAnalysisProcess::LevelAnalysisProcess() : m_decoder(new QAudioDecoder(this))
{
    QAudioFormat format;
    format.setSampleFormat(QAudioFormat::SampleFormat::Float);
    m_decoder->setAudioFormat(format);

    connect(m_decoder, &QAudioDecoder::bufferReady, this, &LevelAnalysisProcess::bufferReady);
    connect(m_decoder, &QAudioDecoder::finished, this, &LevelAnalysisProcess::decodeFinished);
    connect(m_decoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error),
            this, [this](QAudioDecoder::Error error) {
        // Analysis is best-effort: finish with whatever was decoded so far rather
        // than leaving the caller waiting on a completed() that never comes.
        qWarning() << "LevelAnalysisProcess: decoder error" << error << m_decoder->errorString();
        if(m_done)
            return;
        m_done = true;
        flushHop();
        complete();
    });
}

void LevelAnalysisProcess::startProcessing()
{
    qDebug() << "LevelAnalysisProcess: starting analysis of" << audioFilePath();
    m_decoder->setSource(audioFilePath());
    m_decoder->start();
}

void LevelAnalysisProcess::bufferReady()
{
    if(m_done)
        return;

    const QAudioBuffer buffer = m_decoder->read();

    if(m_sampleRate == 0)
    {
        m_sampleRate = buffer.format().sampleRate();
        m_channelCount = buffer.format().channelCount();
        if(m_sampleRate <= 0 || m_channelCount <= 0)
        {
            m_sampleRate = 0;   // try again on the next buffer
            return;
        }

        // ~300 Hz: fine enough that the band tracks read as an actual band-limited
        // waveform (real transient peaks) rather than a smoothed energy curve.
        m_hopSize = std::max(1, m_sampleRate / 300);

        // Butterworth (Q=0.707, maximally flat) low/high split at 250Hz/4kHz, with
        // a wider bandpass roughly covering the gap between them for "mid". Not
        // meant to be precise - just a usable low/mid/high feel for effects.
        m_lowFilter.setLowpass(m_sampleRate, 250.0, 0.707);
        m_highFilter.setHighpass(m_sampleRate, 4000.0, 0.707);
        m_midFilter.setBandpass(m_sampleRate, 1000.0, 0.5);
    }

    const float *data = buffer.constData<float>();
    const int frameCount = buffer.frameCount();
    const int channels = m_channelCount;

    for(int i = 0; i < frameCount; ++i)
    {
        float mono = 0.0f;
        for(int c = 0; c < channels; ++c)
            mono += data[i * channels + c];
        mono /= static_cast<float>(channels);

        processSample(mono);
    }
}

void LevelAnalysisProcess::processSample(float t_mono)
{
    const float low = m_lowFilter.process(t_mono);
    const float mid = m_midFilter.process(t_mono);
    const float high = m_highFilter.process(t_mono);

    m_levelSumSq += double(t_mono) * t_mono;
    m_lowPeak  = std::max(m_lowPeak,  std::abs(low));
    m_midPeak  = std::max(m_midPeak,  std::abs(mid));
    m_highPeak = std::max(m_highPeak, std::abs(high));

    if(++m_hopCount >= m_hopSize)
        flushHop();
}

void LevelAnalysisProcess::flushHop()
{
    if(m_hopCount == 0)
        return;

    m_levelSamples.append(static_cast<float>(std::sqrt(m_levelSumSq / m_hopCount)));
    m_lowSamples.append(m_lowPeak);
    m_midSamples.append(m_midPeak);
    m_highSamples.append(m_highPeak);

    m_levelSumSq = 0.0;
    m_lowPeak = m_midPeak = m_highPeak = 0.0f;
    m_hopCount = 0;
}

void LevelAnalysisProcess::decodeFinished()
{
    if(m_done)
        return;
    m_done = true;

    flushHop();   // don't discard a trailing partial hop
    complete();
}

void LevelAnalysisProcess::processingComplete()
{
    SongData *songData = sequence()->songData();
    const double featureRate = (m_sampleRate > 0 && m_hopSize > 0)
                                    ? double(m_sampleRate) / m_hopSize
                                    : 100.0;

    songData->addFeature(SongData::FeatureLevel, featureRate).setSamples(m_levelSamples);
    songData->addFeature(SongData::FeatureLow, featureRate).setSamples(m_lowSamples);
    songData->addFeature(SongData::FeatureMid, featureRate).setSamples(m_midSamples);
    songData->addFeature(SongData::FeatureHigh, featureRate).setSamples(m_highSamples);
    songData->setSourcePath(sequence()->filePath());

    qDebug() << "LevelAnalysisProcess: complete -" << m_levelSamples.size()
             << "feature samples at" << featureRate << "Hz";
}

AudioProcessorInformation LevelAnalysisProcess::info()
{
    AudioProcessorInformation toReturn([](){return new LevelAnalysisProcess;});
    toReturn.name = "Levels & Frequency";
    toReturn.id = "photon.audio-process.levels";

    return toReturn;
}

} // namespace photon
