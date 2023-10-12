#ifndef PHOTON_VAMP_H
#define PHOTON_VAMP_H
#include <vamp-hostsdk/PluginHostAdapter.h>
#include <vamp-hostsdk/PluginInputDomainAdapter.h>
#include <vamp-hostsdk/PluginLoader.h>
#include <QObject>
#include <QAudioDecoder>
#include <QBuffer>
#include <QUrl>

namespace photon {

using Vamp::Plugin;
using Vamp::PluginHostAdapter;
using Vamp::RealTime;
using Vamp::HostExt::PluginLoader;
using Vamp::HostExt::PluginWrapper;
using Vamp::HostExt::PluginInputDomainAdapter;

#define HOST_VERSION "1.5"

enum Verbosity {
    PluginIds,
    PluginOutputIds,
    PluginInformation,
    PluginInformationDetailed
};

class Vamp : public QObject
{
    Q_OBJECT
public:
    Vamp();

    void printPluginCategoryList();

    void setPluginName(const QString &);
    void setPluginId(const QString &);
    void setFile(const QUrl &);

    void start();

    const QList<float> &beats();

signals:
    void processingComplete();

public slots:
    void loadAudio(const QUrl &);

private slots:
    void bufferReady();
    void finished();
    void audioError(QAudioDecoder::Error error);

private:
    bool initPlugin();
    bool startReading(QAudioBuffer &);
    void processPlugin(int frames);
    QList<float> m_beats;
    QAudioDecoder *m_decoder;
    Plugin::OutputList m_outputs;
    Plugin::OutputDescriptor m_od;
    Plugin::FeatureSet m_features;
    RealTime m_rt;
    RealTime m_adjustment;
    QString m_pluginName;
    QString m_pluginId;
    QUrl m_file;
    int m_channelCount = 0;
    int m_sampleRate = 0;
    int m_blockSize;
    int m_stepSize;
    int m_overlapSize;
    int m_outputNo = 1;
    int m_finalStepsRemaining;
    int m_count = 0;
    qint64 m_currentStep = 0;
    bool m_started = false;
    bool m_useFrames = false;
    Plugin *m_plugin = nullptr;
    QBuffer *m_out;
    std::vector<float> m_cache;
    float *m_filebuf;
    float **m_plugbuf;

};

} // namespace photon

#endif // PHOTON_VAMP_H
