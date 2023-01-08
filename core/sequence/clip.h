#ifndef PHOTON_CLIP_H
#define PHOTON_CLIP_H

#include <QObject>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT Clip : public QObject
{
    Q_OBJECT
public:
    explicit Clip(QObject *parent = nullptr);
    Clip(double t_start, double t_duration, QObject *parent = nullptr);
    ~Clip();

    QString name() const;
    Sequence *sequence() const;
    ClipLayer *layer() const;
    void processChannels(ProcessContext &);
    Channel *channelAtIndex(int index) const;
    int channelCount() const;

    void setMask(FixtureMask *);
    FixtureMask *mask() const;

    void addFalloffEffect(FalloffEffect *);
    void removeFalloffEffect(FalloffEffect *);
    FalloffEffect *falloffEffectAtIndex(int index) const;
    int falloffEffectCount() const;
    void setRoutine(Routine *);
    Routine *routine() const;

    void setStartTime(double);
    void setEndTime(double);
    void setDuration(double);

    double startTime() const;
    double endTime() const;
    double duration() const;

    void restore(Project &);
    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

public slots:
    void channelUpdatedSlot(photon::Channel *);
    void falloffUpdatedSlot(photon::FalloffEffect *);
    void channelAddedSlot(int index);
    void channelRemovedSlot(int index);
    void routineChannelUpdatedSlot(int index);

signals:

    void falloffEffectAdded(photon::FalloffEffect*);
    void falloffEffectRemoved(photon::FalloffEffect*);
    void maskChanged(photon::FixtureMask *);
    void routineChanged(photon::Routine *);
    void falloffMapChanged(photon::FixtureFalloffMap *);
    void clipUpdated(photon::Clip *);
    void channelUpdated(photon::Channel *);
    void channelAdded(photon::Channel *);
    void channelRemoved(photon::Channel *);
    void channelMoved(photon::Channel *);
    void falloffUpdated(photon::FalloffEffect *);

private:
    friend class ClipLayer;
    friend class Sequence;

    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_CLIP_H
