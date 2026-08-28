#ifndef PHOTON_GRAPHCONTEXTNODE_H
#define PHOTON_GRAPHCONTEXTNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

struct RoutineEvaluationContext;

// The single, generic "Globals" node auto-created inside every subgraph. It
// exposes the host-supplied per-frame context (time, fixture, resolution, …) as
// output ports that inner nodes wire from. A graph only gets the ports that make
// sense for it: configure() is called with the relevant ids so the node never
// carries a port the graph can't provide. The port ids match the ones the old
// per-domain globals nodes used, so existing saved connections still resolve.
//
// On fresh construction the enclosing subgraph calls configure(); on load the
// library creates a bare node and readFromJson restores its ports.
class PHOTONCORE_EXPORT GraphContextNode : public keira::Node
{
public:
    // Well-known context port ids.
    const static QByteArray DMXMatrixPort;
    const static QByteArray FixtureListPort;
    const static QByteArray FixturePort;
    const static QByteArray FixtureIndexPort;
    const static QByteArray FixtureTotalPort;
    const static QByteArray RelativeTimePort;
    const static QByteArray GlobalTimePort;
    const static QByteArray TimeOffsetPort;
    const static QByteArray WidthPort;
    const static QByteArray HeightPort;
    const static QByteArray PixelIndexPort;
    const static QByteArray PixelGlobalIndexPort;
    const static QByteArray PixelTotalPort;

    GraphContextNode();

    // Add the given context ports (in order), creating the right parameter type
    // for each id. Called by the enclosing subgraph on fresh construction, and
    // again after loading - ports that already exist are left alone, so a graph
    // saved before a new port existed picks it up on next load rather than being
    // stuck without it forever.
    void configure(const QByteArrayList &portIds);

    // Set the ports this node actually has from the evaluation context. Only the
    // host-derived ports (time/fixture/resolution) are set here; domain-specific
    // ports (fixture list, pixel indices) are pushed by the subgraph directly.
    void fillFromContext(const RoutineEvaluationContext &) const;

    // Convenience port sets for each graph domain.
    static QByteArrayList fixturePorts();
    static QByteArrayList canvasPorts();
    static QByteArrayList pixelPorts();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    keira::Parameter *addPort(const QByteArray &id);
};

} // namespace photon

#endif // PHOTON_GRAPHCONTEXTNODE_H
