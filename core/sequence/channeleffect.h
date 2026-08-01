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

struct ChannelEffectViewState
{
    double yScale = -1.0;
    double yOffset = -1.0;

    ChannelEffectViewState(){}
    ChannelEffectViewState(double scale, double offset):yScale(scale),yOffset(offset){}
};


class PHOTONCORE_EXPORT ChannelEffect
{
public:
    ChannelEffect(const QByteArray &id = QByteArray{});
    virtual ~ChannelEffect();

    QByteArray id() const;
    QByteArray uniqueId() const;
    void setId(const QByteArray &);
    void setName(const QString &);
    QString name() const;

    Channel *channel() const;
    int index() const;
    virtual float * process(float *value, uint size, double time) const;
    virtual double processTime(double time) const;

    // Whether this effect's own contribution can be meaningfully isolated as
    // (chain with it) - (chain without it) and drawn as the gizmo's cyan preview
    // curve, centred on zero. True only for additive generators (sine, sawtooth,
    // ...); false for time-warps and modifiers where that difference isn't a clean
    // standalone shape. Off by default.
    virtual bool providesIsolatedContribution() const { return false; }
    virtual ChannelEffectEditor *createEditor() {return new ChannelEffectEditor(this);}
    void updated();

    ChannelEffectViewState viewState() const;
    void setViewState(const ChannelEffectViewState &);

    ChannelEffect *previousEffect() const;
    virtual void restore(Project &);
    virtual void readFromJson(const QJsonObject &);
    virtual void writeToJson(QJsonObject &) const;

protected:

    virtual void addedToChannel();

private:
    friend class Channel;
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CHANNELEFFECT_H
