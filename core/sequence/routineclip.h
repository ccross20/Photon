#ifndef PHOTON_ROUTINECLIP_H
#define PHOTON_ROUTINECLIP_H
#include "fixtureclip.h"

namespace photon {

class PHOTONCORE_EXPORT RoutineClip : public FixtureClip
{
    Q_OBJECT
public:
    RoutineClip(QObject *parent = nullptr);
    RoutineClip(double start, double duration, QObject *parent = nullptr);
    ~RoutineClip();

    void processChannels(ProcessContext &) override;
    void setRoutine(Routine *);
    Routine *routine() const;

    void restore(Project &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

public slots:
    void routineChannelUpdatedSlot(int index);
    void channelAddedSlot(int index);
    void channelRemovedSlot(int index);

signals:
    void routineChanged(photon::Routine *);

private:
    class Impl;
    Impl *m_impl;


};

} // namespace photon

#endif // PHOTON_ROUTINECLIP_H
