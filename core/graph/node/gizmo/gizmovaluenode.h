#ifndef GIZMOVALUENODE_H
#define GIZMOVALUENODE_H
#include "model/node.h"
#include "photon-global.h"
#include "model/parameter/stringoptionparameter.h"
#include "model/parameter/anyparameter.h"

namespace photon {

// Generic gizmo value source. Reads any gizmo's published output from the
// surface value bus, with no per-gizmo-type node or dynamic cast. Replaces the
// type-specific SliderGizmoNode/ToggleGizmoNode pattern.
//
// A single "Source" selector lists every gizmo->output pair on the surface; its
// value is the bus key "<uniqueId>/<portId>". (A dependent gizmo+port pair of
// dropdowns can't work here: NodeEditor never rebuilds a parameter widget when
// another parameter changes.)
class PHOTONCORE_EXPORT GizmoValueNode : public keira::Node
{
public:
    const static QByteArray Source;
    const static QByteArray Value;

    GizmoValueNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

protected:
    void addedToGraph(keira::Graph*) override;

private:
    keira::StringOptionParameter *m_sourceParam = nullptr;
    keira::AnyParameter *m_valueParam = nullptr;
};

} // namespace photon

#endif // GIZMOVALUENODE_H
