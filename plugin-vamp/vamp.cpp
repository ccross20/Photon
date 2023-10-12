#include "vamp.h"

#include <QDebug>
#include <QUrl>
#include <iostream>
#include <fstream>
#include <set>

#include <cstring>
#include <cstdlib>

#include <cmath>

namespace photon {

Vamp::Vamp():
m_decoder(new QAudioDecoder(this))
{
    QAudioFormat format;
    format.setSampleFormat(QAudioFormat::SampleFormat::Float);

    m_decoder->setAudioFormat(format);

    connect(m_decoder, &QAudioDecoder::bufferReady, this, &Vamp::bufferReady);
    connect(m_decoder, &QAudioDecoder::finished, this, &Vamp::finished);
    connect(m_decoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error),
        this,&Vamp::audioError);

}


void Vamp::loadAudio(const QUrl &path)
{

    m_decoder->setSource(path);
    m_decoder->start();
}

void Vamp::audioError(QAudioDecoder::Error error)
{
    qDebug() << error;
}


const QList<float> &Vamp::beats()
{
    return m_beats;
}


static double
toSeconds(const RealTime &time)
{
    return time.sec + double(time.nsec + 1) / 1000000000.0;
}

void
printFeatures(int frame, int sr,
              const Plugin::OutputDescriptor &output, int outputNo,
              const Plugin::FeatureSet &features, QBuffer *out, bool useFrames, QList<float> &beats)
{
    QDataStream stream(out);
    static int featureCount = -1;

    if (features.find(outputNo) == features.end()) return;

    for (size_t i = 0; i < features.at(outputNo).size(); ++i) {

        const Plugin::Feature &f = features.at(outputNo).at(i);

        bool haveRt = false;
        RealTime m_rt;

        if (output.sampleType == Plugin::OutputDescriptor::VariableSampleRate) {
            m_rt = f.timestamp;
            haveRt = true;
        } else if (output.sampleType == Plugin::OutputDescriptor::FixedSampleRate) {
            int n = featureCount + 1;
            if (f.hasTimestamp) {
                n = int(round(toSeconds(f.timestamp) * output.sampleRate));
            }
            m_rt = RealTime::fromSeconds(double(n) / output.sampleRate);
            haveRt = true;
            featureCount = n;
        }

        if (useFrames) {

            int displayFrame = frame;

            if (haveRt) {
                displayFrame = RealTime::realTime2Frame(m_rt, sr);
            }


            if (f.hasDuration) {
                displayFrame = RealTime::realTime2Frame(f.duration, sr);
            }

            beats << displayFrame;

        } else {

            if (!haveRt) {
                m_rt = RealTime::frame2RealTime(frame, sr);
            }


            if (f.hasDuration) {
                m_rt = f.duration;
            }

            beats << m_rt.sec + m_rt.msec()/1000.0f;

        }
/*
        for (unsigned int j = 0; j < f.values.size(); ++j) {
            qDebug() << " " << f.values[j];
        }
        qDebug() << " " << QString::fromStdString(f.label);
*/

    }
}

void Vamp::bufferReady()
{
    auto buffer = m_decoder->read();
    if(!m_started)
        startReading(buffer);

    m_count += buffer.frameCount();

    auto data = buffer.constData<float>();
    for(int i = 0; i < buffer.sampleCount(); ++i)
        m_cache.push_back(*data++);

    int counter = 0;
    while(m_count >= m_blockSize)
    {
        processPlugin(m_blockSize);
        ++counter;
        m_cache.erase(m_cache.begin(), m_cache.begin() +  (m_stepSize * m_channelCount));
    }

    //m_cache.erase(m_cache.begin(), m_cache.begin() + ((counter-1) * m_stepSize) + m_overlapSize);

}

void Vamp::processPlugin(int t_frames)
{

    int count = t_frames;

    for (int c = 0; c < m_channelCount; ++c) {
        int j = 0;
        while (j < count) {
            m_plugbuf[c][j] = m_cache[j * m_channelCount + c];
            ++j;
        }
        while (j < m_blockSize) {
            m_plugbuf[c][j] = 0.0f;
            ++j;
        }
    }

    m_rt = RealTime::frame2RealTime(m_currentStep * m_stepSize, m_sampleRate);

    m_features = m_plugin->process(m_plugbuf, m_rt);

    printFeatures
        (RealTime::realTime2Frame(m_rt + m_adjustment, m_sampleRate),
         m_sampleRate, m_od, m_outputNo, m_features, m_out, m_useFrames,m_beats);



    m_currentStep += 1;
    m_count -= m_stepSize;
}

void Vamp::finished()
{

    processPlugin(m_count);
    m_rt = RealTime::frame2RealTime(m_currentStep * m_stepSize, m_sampleRate);

    m_features = m_plugin->getRemainingFeatures();

    printFeatures
        (RealTime::realTime2Frame(m_rt + m_adjustment, m_sampleRate),
         m_sampleRate, m_od, m_outputNo, m_features, m_out, m_useFrames, m_beats);

    emit processingComplete();
}

bool Vamp::startReading(QAudioBuffer &buffer)
{
    m_started = true;

    if(m_channelCount == 0)// initialize
    {
        m_channelCount = buffer.format().channelCount();
        m_sampleRate = buffer.format().sampleRate();
    }

    if(!initPlugin())
        return false;

    int channels = m_channelCount;

    m_filebuf = new float[m_blockSize * channels];
    m_plugbuf = new float*[channels];
    for (int c = 0; c < channels; ++c) m_plugbuf[c] = new float[m_blockSize + 2];


    // The channel queries here are for informational purposes only --
    // a PluginChannelAdapter is being used automatically behind the
    // scenes, and it will take case of any channel mismatch


    m_outputs = m_plugin->getOutputDescriptors();

    PluginWrapper *wrapper = 0;
    m_adjustment = RealTime::zeroTime;

    if (m_outputs.empty()) {
        qDebug() << "ERROR: Plugin has no m_outputs!";
        return false;
    }

    m_outputNo = 0;
    QString output = "test";

    if (m_outputNo < 0) {

        for (size_t oi = 0; oi < m_outputs.size(); ++oi) {
            if (QString::fromStdString(m_outputs[oi].identifier) == output) {
                m_outputNo = oi;
                break;
            }
        }

        if (m_outputNo < 0) {
            qDebug() << "ERROR: Non-existent output \"" << output << "\" requested";
            return false;
        }

    } else {

        if (int(m_outputs.size()) <= m_outputNo) {
            qDebug() << "ERROR: Output " << m_outputNo << " requested, but m_plugin has only " << m_outputs.size() << " output(s)";
            return false;
        }
    }

    m_od = m_outputs[m_outputNo];

    if (!m_plugin->initialise(channels, m_stepSize, m_blockSize)) {
        qDebug() << "ERROR: Plugin initialise (channels = " << channels
             << ", m_stepSize = " << m_stepSize << ", m_blockSize = "
             << m_blockSize << ") failed.";
        return false;
    }

    wrapper = dynamic_cast<PluginWrapper *>(m_plugin);
    if (wrapper) {
        // See documentation for
        // PluginInputDomainAdapter::getTimestampAdjustment
        PluginInputDomainAdapter *ida =
            wrapper->getWrapper<PluginInputDomainAdapter>();
        if (ida) m_adjustment = ida->getTimestampAdjustment();
    }

    return true;
}

void Vamp::setPluginName(const QString &t_value)
{
    m_pluginName = t_value;
}

void Vamp::setPluginId(const QString &t_value)
{
    m_pluginId = t_value;
}

void Vamp::setFile(const QUrl &t_value)
{
    m_file = t_value;
}


void Vamp::start()
{
    loadAudio(m_file);
}


bool Vamp::initPlugin()
{

    QByteArray outArray;
    m_out = new QBuffer(&outArray);
    m_out->open(QBuffer::WriteOnly);


    PluginLoader *loader = PluginLoader::getInstance();

    PluginLoader::PluginKey key = loader->composePluginKey(m_pluginName.toStdString(), m_pluginId.toStdString());

    m_plugin = loader->loadPlugin
        (key, m_sampleRate, PluginLoader::ADAPT_ALL_SAFE);
    if (!m_plugin) {
        qDebug() << ": ERROR: Failed to load m_plugin \"" << m_pluginId
             << "\" from library \"" << m_pluginName << "\"";

        return false;
    }

    // Note that the following would be much simpler if we used a
    // PluginBufferingAdapter as well -- i.e. if we had passed
    // PluginLoader::ADAPT_ALL to loader->loadPlugin() above, instead
    // of ADAPT_ALL_SAFE.  Then we could simply specify our own block
    // size, keep the step size equal to the block size, and ignore
    // the m_plugin's bleatings.  However, there are some issues with
    // using a PluginBufferingAdapter that make the results sometimes
    // technically different from (if effectively the same as) the
    // un-adapted m_plugin, so we aren't doing that here.  See the
    // PluginBufferingAdapter documentation for details.

    m_blockSize = m_plugin->getPreferredBlockSize();
    m_stepSize = m_plugin->getPreferredStepSize();

    if (m_blockSize == 0) {
        m_blockSize = 1024;
    }
    if (m_stepSize == 0) {
        if (m_plugin->getInputDomain() == Plugin::FrequencyDomain) {
            m_stepSize = m_blockSize/2;
        } else {
            m_stepSize = m_blockSize;
        }
    } else if (m_stepSize > m_blockSize) {
        qDebug() << "WARNING: m_stepSize " << m_stepSize << " > m_blockSize " << m_blockSize << ", resetting m_blockSize to ";
        if (m_plugin->getInputDomain() == Plugin::FrequencyDomain) {
            m_blockSize = m_stepSize * 2;
        } else {
            m_blockSize = m_stepSize;
        }
        qDebug() << m_blockSize;
    }
    m_overlapSize = m_blockSize - m_stepSize;
    m_currentStep = 0;
    m_finalStepsRemaining = std::max(1, (m_blockSize / m_stepSize) - 1); // at end of file, this many part-silent frames needed after we hit EOF



/*
    m_rt = RealTime::frame2RealTime(m_currentStep * m_stepSize, m_sampleRate);

    m_features = m_plugin->getRemainingFeatures();

    printFeatures(RealTime::realTime2Frame(m_rt + m_adjustment, m_sampleRate),
                  m_sampleRate, m_od, outputNo, m_features, m_out, useFrames);
                  */



    return true;
}




void Vamp::printPluginCategoryList()
{
    PluginLoader *loader = PluginLoader::getInstance();

    for(auto str : PluginHostAdapter::getPluginPath())
        qDebug() << QString::fromStdString(str);

    std::vector<PluginLoader::PluginKey> plugins = loader->listPlugins();

    qDebug() << "Plugin count:" << plugins.size();

    for (size_t i = 0; i < plugins.size(); ++i) {

        PluginLoader::PluginKey key = plugins[i];

        PluginLoader::PluginCategoryHierarchy category =
            loader->getPluginCategory(key);

        Plugin *m_plugin = loader->loadPlugin(key, 48000);
        if (!m_plugin) continue;

        qDebug() << "Plugin: " << QString::fromStdString(m_plugin->getName()) << QString::fromStdString(key);

        //std::cout << catstr << key << ":::" << m_plugin->getName() << ":::" << m_plugin->getMaker() << ":::" << m_plugin->getDescription() << std::endl;
    }
}



} // namespace photon
