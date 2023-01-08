#ifndef PHOTON_CHANNELEFFECT_H
#define PHOTON_CHANNELEFFECT_H
#include <QWidget>
#include "photon-global.h"
#include "viewer/channeleffecteditor.h"

namespace photon {

struct EffectInformation
{

    EffectInformation(){}
    EffectInformation(std::function<ChannelEffect*()> _callback):callback(_callback){}

    QByteArray effectId;
    QByteArray translateId;
    QByteArray iconId;
    QString name;
    std::function<ChannelEffect*()> callback;

    CategoryList categories;
};

class PHOTONCORE_EXPORT ChannelEffect
{
public:
    ChannelEffect(const QByteArray &id = QByteArray{});
    virtual ~ChannelEffect();

    QByteArray id() const;
    void setId(const QByteArray &);
    void setName(const QString &);
    QString name() const;

    Channel *channel() const;
    int index() const;
    virtual double process(double value, double time) const = 0;
    virtual ChannelEffectEditor *createEditor() {return new ChannelEffectEditor(this);}
    void updated();

    ChannelEffect *previousEffect() const;
    virtual void restore(Project &);
    virtual void readFromJson(const QJsonObject &);
    virtual void writeToJson(QJsonObject &) const;

private:
    friend class Channel;
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CHANNELEFFECT_H
