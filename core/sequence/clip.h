#ifndef PHOTON_CLIP_H
#define PHOTON_CLIP_H

#include <QObject>
#include <QEasingCurve>
#include "photon-global.h"
#include "channel.h"

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

class PHOTONCORE_EXPORT Clip : public QObject
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
    bool timeIsValid(double) const;
    QByteArray type() const;    

    void addMaskEffect(MaskEffect *);
    void removeMaskEffect(MaskEffect *);
    MaskEffect *maskEffectAtIndex(int index) const;
    int maskEffectCount() const;
    const QVector<Fixture*> maskedFixtures() const;
    virtual QWidget *widget() const;

    void addFalloffEffect(FalloffEffect *);
    void removeFalloffEffect(FalloffEffect *);
    FalloffEffect *falloffEffectAtIndex(int index) const;
    int falloffEffectCount() const;
    void setDefaultFalloff(double);
    double defaultFalloff() const;
    double falloff(Fixture *t_fixture) const;

    void addClipEffect(ClipEffect *);
    void removeClipEffect(ClipEffect *);
    ClipEffect *clipEffectAtIndex(int index) const;
    int clipEffectCount() const;

    void setStartTime(double);
    void setEndTime(double);
    void setDuration(double);

    double startTime() const;
    double endTime() const;
    double duration() const;
    double strengthAtTime(double) const;

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

    void markChanged();

protected:
    virtual void startTimeUpdated(double);
    virtual void durationUpdated(double);
    void setType(const QByteArray t_type);


public slots:
    void falloffUpdatedSlot(photon::FalloffEffect *);
    void channelUpdatedSlot(photon::Channel *);
    void maskUpdatedSlot(photon::MaskEffect *);
    void clipEffectUpdatedSlot(photon::ClipEffect *);
    photon::Channel *addChannel(const photon::ChannelInfo &info = ChannelInfo{}, int index = -1);
    void removeChannel(int index);

signals:

    void falloffEffectAdded(photon::FalloffEffect*);
    void falloffEffectRemoved(photon::FalloffEffect*);
    void maskAdded(photon::MaskEffect *);
    void maskRemoved(photon::MaskEffect *);
    void maskUpdated(photon::MaskEffect *);
    void maskMoved(photon::MaskEffect *);
    void clipEffectAdded(photon::ClipEffect *);
    void clipEffectRemoved(photon::ClipEffect *);
    void clipEffectUpdated(photon::ClipEffect *);
    void clipEffectMoved(photon::ClipEffect *);
    void falloffMapChanged(photon::FixtureFalloffMap *);
    void clipUpdated(photon::Clip *);
    void falloffUpdated(photon::FalloffEffect *);
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
