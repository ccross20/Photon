#ifndef PHOTON_CLIP_H
#define PHOTON_CLIP_H

#include <QObject>
#include <QEasingCurve>
#include "photon-global.h"
#include "channel.h"
#include "baseeffectparent.h"

namespace photon {

struct ClipInformation
{

    ClipInformation(){}
    ClipInformation(std::function<Clip*()> _callback):callback(_callback){}

    QByteArray id;
    QByteArray translateId;
    QByteArray iconId;
    QString name;
    bool isCanvas = false;
    std::function<Clip*()> callback;

    CategoryList categories;
};

class PHOTONCORE_EXPORT Clip : public QObject, public BaseEffectParent
{
    Q_OBJECT
public:
    explicit Clip(QObject *parent = nullptr);
    Clip(double t_start, double t_duration, QObject *parent = nullptr);
    virtual ~Clip();

    QString name() const;
    void setName(const QString &);
    void setId(const QByteArray&);
    QByteArray id() const;
    QByteArray uniqueId() const;
    Sequence *sequence() const;
    ClipLayer *layer() const;
    virtual void processChannels(ProcessContext &);
    virtual bool timeIsValid(double) const;
    QByteArray type() const;
    virtual QWidget *widget() const;

    ChannelParameterContainer *parameters() const;
    void addChannelParameter(ChannelParameter *);
    void removeChannelParameter(ChannelParameter *);
    const QVector<Channel*> channelsForParameter(ChannelParameter *) const;

    void addClipEffect(BaseEffect *) override;
    void removeClipEffect(BaseEffect *) override;
    BaseEffect *clipEffectAtIndex(int index) const override;
    int clipEffectCount() const override;
    const QVector<BaseEffect*> &clipEffects() const override;

    void setStartTime(double);
    void setEndTime(double);
    void setDuration(double);

    double startTime() const override;
    double endTime() const;
    double duration() const override;
    double strengthAtTime(double) const override;

    Channel *channelAtIndex(int index) const;
    int channelCount() const;
    void clearChannels();
    const QVector<Channel*> channels() const;

    double strength() const;
    double easeInDuration() const;
    double easeOutDuration() const;
    QEasingCurve::Type easeInType() const;
    QEasingCurve::Type easeOutType() const;

    void setStrength(double);
    void setEaseInDuration(double);
    void setEaseOutDuration(double);
    void setEaseInType(QEasingCurve::Type);
    void setEaseOutType(QEasingCurve::Type);

    virtual void restore(Project &);
    virtual void readFromJson(const QJsonObject &, const LoadContext &);
    virtual void writeToJson(QJsonObject &) const;

    void markChanged() override;
    void effectUpdated(photon::BaseEffect *) override;

protected:
    virtual void startTimeUpdated(double);
    virtual void durationUpdated(double);
    virtual void layerDidChange(Layer *);
    void setType(const QByteArray t_type);


public slots:
    void channelUpdatedSlot(photon::Channel *);
    void effectUpdatedSlot(photon::BaseEffect *);
    photon::Channel *addChannel(const photon::ChannelInfo &info = ChannelInfo{}, int index = -1);
    void removeChannel(int index);
    void createChannelsFromParameter(ChannelParameter *, ChannelInfo::ChannelType type = ChannelInfo::ChannelTypeNumber);

signals:

    void clipEffectAdded(photon::BaseEffect *);
    void clipEffectRemoved(photon::BaseEffect *);
    void clipEffectUpdated(photon::BaseEffect *);
    void clipEffectMoved(photon::BaseEffect *);
    void clipUpdated(photon::Clip *);
    void channelUpdated(photon::Channel *);
    void channelAdded(photon::Channel *);
    void channelRemoved(photon::Channel *);
    void channelMoved(photon::Channel *);

private:
    friend class ClipLayer;
    friend class Sequence;

    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_CLIP_H
