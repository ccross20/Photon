#ifndef PHOTON_CLIPEFFECT_H
#define PHOTON_CLIPEFFECT_H

#include "photon-global.h"
#include "channel.h"
#include "sequence.h"
#include "channel/parameter/channelparametercontainer.h"

namespace photon {

struct ClipEffectInformation
{

    ClipEffectInformation(){}
    ClipEffectInformation(std::function<ClipEffect*()> _callback):callback(_callback){}

    QByteArray id;
    QByteArray translateId;
    QByteArray iconId;
    QString name;
    bool isCanvas = false;
    std::function<ClipEffect*()> callback;

    CategoryList categories;
};

class PHOTONCORE_EXPORT ClipEffectEvaluationContext
{
public:
    ClipEffectEvaluationContext(ProcessContext &context):dmxMatrix(context.dmxMatrix),
        project(context.project),
        globalTime(context.globalTime){}
    virtual ~ClipEffectEvaluationContext(){}

    DMXMatrix &dmxMatrix;
    Project *project = nullptr;
    QHash<QByteArray,QVariant> channelValues;
    double relativeTime = 0.0;
    double globalTime = 0.0;
    double delayTime = 0.0;
    double strength = 1.0;
};

class PHOTONCORE_EXPORT ClipEffect : public QObject
{
    Q_OBJECT
public:
    ClipEffect(const QByteArray &t_id = QByteArray());
    virtual ~ClipEffect();

    virtual void init();
    virtual void processChannels(ProcessContext &);

    QByteArray id() const;
    QByteArray uniqueId() const;
    void setId(const QByteArray &);
    void setName(const QString &);
    QString name() const;
    void updated();
    virtual QWidget *createEditor();

    double getNumberAtTime(const QByteArray &id, double) const;
    QColor getColorAtTime(const QByteArray &id, double) const;

    Channel *channelAtIndex(int index) const;
    int channelCount() const;
    void clearChannels();
    const QVector<Channel*> channels() const;
    const QVector<Channel*> channelsForParameter(ChannelParameter *) const;

    ChannelParameterContainer *parameters() const;
    void addChannelParameter(ChannelParameter *);
    void removeChannelParameter(ChannelParameter *);

    Clip *clip() const;
    virtual void restore(Project &);
    virtual void readFromJson(const QJsonObject &, const LoadContext &);
    virtual void writeToJson(QJsonObject &) const;

protected:
    virtual void addedToClip(Clip*);
    virtual void layerChanged(Layer*);
    virtual void startTimeUpdated(double);
    virtual void durationUpdated(double);
    void prepareContext(ClipEffectEvaluationContext &) const;

public slots:
    photon::Channel *addChannel(const photon::ChannelInfo &info = ChannelInfo{}, int index = -1);
    void removeChannel(int index);
    void channelUpdatedSlot(photon::Channel *);

    void createChannelsFromParameter(ChannelParameter *, ChannelInfo::ChannelType type = ChannelInfo::ChannelTypeNumber);

signals:

    void channelUpdated(photon::Channel *);
    void channelAdded(photon::Channel *);
    void channelRemoved(photon::Channel *);
    void channelMoved(photon::Channel *);

private:
    friend class Clip;
    friend class CanvasLayerGroup;
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CLIPEFFECT_H
