#ifndef PHOTON_SPATIALFALLOFFNODE_H
#define PHOTON_SPATIALFALLOFFNODE_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/stringoptionparameter.h"
#include "photon-global.h"

namespace photon {

class FixtureListParameter;

// Assigns a per-fixture time offset from each fixture's position along a Linear
// Falloff helper object's line.
//
//  - Bounded:   the fixture's projected position on the line, normalised by the
//               helper's length and clamped to [0, 1]. Fixtures before the start
//               get 0, past the end get 1.
//  - Unbounded: the helper is only a direction. The fixture furthest along the
//               negative axis gets 0, the furthest along the positive gets 1,
//               and the rest interpolate linearly between those two extremes.
//
// Multiplier scales every resulting offset (default 1).
class SpatialFalloffNode : public keira::Node
{
public:
    enum Mode { ModeBounded = 0, ModeUnbounded = 1 };

    SpatialFalloffNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    FixtureListParameter *m_inParam = nullptr;
    keira::StringOptionParameter *m_helperParam = nullptr;
    keira::OptionParameter *m_modeParam = nullptr;
    keira::DecimalParameter *m_multiplierParam = nullptr;
    FixtureListParameter *m_outParam = nullptr;
};

} // namespace photon

#endif // PHOTON_SPATIALFALLOFFNODE_H
