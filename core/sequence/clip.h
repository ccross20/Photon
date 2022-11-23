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
    Clip(double t_start, double t_duration);
    ~Clip();

    QString name() const;
    Sequence *sequence() const;
    Layer *layer() const;
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

    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

public slots:
    void channelUpdatedSlot(photon::Channel *);
    void falloffUpdatedSlot(photon::FalloffEffect *);

signals:

    void falloffEffectAdded(photon::FalloffEffect*);
    void falloffEffectRemoved(photon::FalloffEffect*);
    void maskChanged(photon::FixtureMask *);
    void routineChanged(photon::Routine *);
    void falloffMapChanged(photon::FixtureFalloffMap *);
    void timeChanged(double);
    void durationChanged(double);
    void channelUpdated(photon::Channel *);
    void falloffUpdated(photon::FalloffEffect *);

private:
    friend class Layer;
    friend class Sequence;

    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_CLIP_H
