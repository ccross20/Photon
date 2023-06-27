#ifndef PHOTON_FIXTURECLIP_H
#define PHOTON_FIXTURECLIP_H
#include "clip.h"
#include "state/state.h"

namespace photon {

class FixtureClip : public Clip
{
public:
    FixtureClip();
    FixtureClip(double start, double duration, QObject *parent = nullptr);
    virtual ~FixtureClip();


    State *state() const;

    virtual void processChannels(ProcessContext &) override;
    void restore(Project &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

    static ClipInformation info();
private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTURECLIP_H
