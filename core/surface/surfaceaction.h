#ifndef SURFACEACTION_H
#define SURFACEACTION_H

#include <QObject>
#include "photon-global.h"
#include "sequence/channel.h"
#include "sequence/baseeffectparent.h"

namespace photon {


class PHOTONCORE_EXPORT SurfaceAction : public QObject, public BaseEffectParent
{
    Q_OBJECT
public:
    explicit SurfaceAction(QObject *parent = nullptr);
    virtual ~SurfaceAction();


    QString name() const;
    void setName(const QString &);
    void setId(const QByteArray&);
    QByteArray id() const;
    QByteArray uniqueId() const;
    QByteArray type() const;

    virtual void processChannels(ProcessContext &);
    virtual QWidget *widget() const;
    virtual QWidget *settingsWidget() const;

    ChannelParameterContainer *parameters() const;
    void addChannelParameter(ChannelParameter *);
    void removeChannelParameter(ChannelParameter *);
    const QVector<Channel*> channelsForParameter(ChannelParameter *) const;

    void addClipEffect(BaseEffect *) override;
    void removeClipEffect(BaseEffect *) override;
    BaseEffect *clipEffectAtIndex(int index) const override;
    int clipEffectCount() const override;
    const QVector<BaseEffect*> &clipEffects() const override;


    double startTime() const override;
    double duration() const override;
    double strengthAtTime(double) const override;
    double strength() const;
    void setStrength(double);
    void setDuration(double);

    Channel *channelAtIndex(int index) const;
    int channelCount() const;
    void clearChannels();
    const QVector<Channel*> channels() const;

    virtual void restore(Project &);
    virtual void readFromJson(const QJsonObject &, const LoadContext &);
    virtual void writeToJson(QJsonObject &) const;

    void markChanged() override;
    void effectUpdated(photon::BaseEffect *) override;

public slots:
    void channelUpdatedSlot(photon::Channel *);
    void effectUpdatedSlot(photon::BaseEffect *);
    photon::Channel *addChannel(const photon::ChannelInfo &info = ChannelInfo{}, int index = -1);
    void removeChannel(int index);
    void createChannelsFromParameter(ChannelParameter *, ChannelInfo::ChannelType type = ChannelInfo::ChannelTypeNumber);


protected:
    void setType(const QByteArray t_type);

signals:
    void clipEffectAdded(photon::BaseEffect *);
    void clipEffectRemoved(photon::BaseEffect *);
    void clipEffectUpdated(photon::BaseEffect *);
    void clipEffectMoved(photon::BaseEffect *);
    void actionUpdated(photon::SurfaceAction *);
    void channelUpdated(photon::Channel *);
    void channelAdded(photon::Channel *);
    void channelRemoved(photon::Channel *);
    void channelMoved(photon::Channel *);


private:

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // SURFACEACTION_H
