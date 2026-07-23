#ifndef DMXSUBGRAPHNODE_H
#define DMXSUBGRAPHNODE_H

#include "model/subgraphnode.h"
#include "photon-global.h"

namespace photon {

// A bare DMX pass-through subgraph — the same shape as SurfaceNode (DMX
// input/output ports wrapping an internal graph that runs against the shared
// DMX matrix) but without a Surface. Used to host nodes like Fixture State
// that need to run at a specific point in the bus pipeline, e.g. setting
// fixtures' initial values before the Surface node's controls take over.
class PHOTONCORE_EXPORT DMXSubGraphNode : public keira::SubGraphNode
{
public:
    const static QByteArray InputDMX;
    const static QByteArray OutputDMX;

    DMXSubGraphNode();
    ~DMXSubGraphNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // DMXSUBGRAPHNODE_H
