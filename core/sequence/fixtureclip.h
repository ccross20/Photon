#ifndef PHOTON_FIXTURECLIP_H
#define PHOTON_FIXTURECLIP_H
#include "clip.h"
#include "state/state.h"
#include "routine/routine.h"

namespace photon {

class StateCapability;

class PHOTONCORE_EXPORT FixtureClip : public Clip
{
    Q_OBJECT
public:
    FixtureClip(QObject *parent = nullptr);
    FixtureClip(double start, double duration, QObject *parent = nullptr);
    virtual ~FixtureClip();


    void addMaskEffect(MaskEffect *);
    void removeMaskEffect(MaskEffect *);
    MaskEffect *maskEffectAtIndex(int index) const;
    int maskEffectCount() const;
    const QVector<Fixture*> maskedFixtures() const;

    void addFalloffEffect(FalloffEffect *);
    void removeFalloffEffect(FalloffEffect *);
    FalloffEffect *falloffEffectAtIndex(int index) const;
    int falloffEffectCount() const;
    void setDefaultFalloff(double);
    double defaultFalloff() const;
    double falloff(Fixture *t_fixture) const;

    State *state() const;
    Routine *contentGraph() const override;

    // Expose a capability's channel for timeline control: exposes the capability
    // as an input port on the graph's FixtureStateNode, wires a matching input
    // node into it, and adds a clip channel bound to that input node. After this
    // the clip channel animates the capability over the clip's timeline.
    void exposeCapabilityChannel(StateCapability *, int channelIndex);
    // Reverse of exposeCapabilityChannel: removes the input node feeding the
    // capability's port (which drops the bound channel via sync) and unexposes it.
    void unexposeCapabilityChannel(StateCapability *, int channelIndex);
    // Whether the capability's channel is currently exposed for timeline control.
    bool isCapabilityChannelExposed(StateCapability *, int channelIndex) const;

    virtual void processChannels(ProcessContext &) override;
    virtual bool timeIsValid(double) const override;
    void restore(Project &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

    static ClipInformation info();

public slots:
    void falloffUpdatedSlot(photon::FalloffEffect *);
    void maskUpdatedSlot(photon::MaskEffect *);

signals:

    void falloffEffectAdded(photon::FalloffEffect*);
    void falloffEffectRemoved(photon::FalloffEffect*);
    void maskAdded(photon::MaskEffect *);
    void maskRemoved(photon::MaskEffect *);
    void maskUpdated(photon::MaskEffect *);
    void maskMoved(photon::MaskEffect *);
    void falloffMapChanged(photon::FixtureFalloffMap *);
    void falloffUpdated(photon::FalloffEffect *);

private:
    // Reconcile the clip's channels against the graph's input ports (by portId):
    // add a channel for each input node that lacks one, remove channels whose port
    // is gone. Channel 0 (strength) is preserved.
    void syncChannelsFromGraph();

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTURECLIP_H
