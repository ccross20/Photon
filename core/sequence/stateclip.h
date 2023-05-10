#ifndef PHOTON_STATECLIP_H
#define PHOTON_STATECLIP_H
#include "clip.h"
#include "state/state.h"

namespace photon {


class StateClip : public Clip
{
public:
    StateClip();

    StateClip(double start, double duration, QObject *parent = nullptr);
    ~StateClip();

    void processChannels(ProcessContext &) override;

    void setState(State *);
    State *state() const;

    QString name() const override;

    void restore(Project &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_STATECLIP_H
