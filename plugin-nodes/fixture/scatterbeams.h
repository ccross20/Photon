#ifndef PHOTON_SCATTERBEAMS_H
#define PHOTON_SCATTERBEAMS_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/stringoptionparameter.h"
#include "graph/parameter/matrixparameter.h"

namespace photon {

// Spreads a group of movers across a floor area marked by a Boundary Rectangle
// or Boundary Oval helper object: each fixture gets its own point inside the
// shape, output as a Target Location matrix for Look At Target to aim at.
//
// Lives in a fixture graph, where the subgraph evaluates it once per fixture
// (see FixtureSubGraphNode) - the fixture's position in the group comes from
// the evaluation context's fixtureIndex, and the size of the group from the
// Fixture Count input (wire it from the Globals node's "Total Fixtures").
//
// Each beam steers rather than following a fixed figure: it advances along a
// heading whose turn rate is capped at speed/radius, so the path meanders -
// running nearly straight where the noise driving it is near zero, bending
// hardest where it peaks - while never turning tighter than Curve Radius.
// That means the beam carries its heading between frames, so this node holds
// per-beam state; the fixture subgraph gives every fixture its own clone of
// the node, so that state is private to one beam and safe under the parallel
// per-fixture evaluation.
class ScatterBeams : public keira::Node
{
public:
    ScatterBeams();
    ~ScatterBeams();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    keira::StringOptionParameter *m_areaParam;
    keira::IntegerParameter *m_countParam;
    keira::IntegerParameter *m_seedParam;
    keira::DecimalParameter *m_speedParam;
    keira::DecimalParameter *m_curveRadiusParam;
    MatrixParameter *m_targetParam;

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SCATTERBEAMS_H
