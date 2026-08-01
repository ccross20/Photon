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

// One detected event from a VariableSampleRate output. barPosition carries the
// feature's label parsed as an int (e.g. qm-barbeattracker labels each beat with
// its 1-based position in the bar); 0 when the output doesn't label events.
struct BeatEvent
{
    double time = 0.0;
    int barPosition = 0;
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

    const QList<BeatEvent> &beats();

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
    QList<BeatEvent> m_beats;
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
    int m_blockSize = 0;
    int m_stepSize = 0;
    int m_overlapSize = 0;
    int m_outputNo = 1;
    int m_finalStepsRemaining = 0;
    int m_count = 0;
    qint64 m_currentStep = 0;
    bool m_started = false;
    bool m_failed = false;
    bool m_useFrames = false;
    Plugin *m_plugin = nullptr;
    QBuffer *m_out = nullptr;
    std::vector<float> m_cache;
    float *m_filebuf = nullptr;
    float **m_plugbuf = nullptr;

};

} // namespace photon

#endif // PHOTON_VAMP_H
