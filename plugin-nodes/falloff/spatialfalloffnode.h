#ifndef PHOTON_SPATIALFALLOFFNODE_H
#define PHOTON_SPATIALFALLOFFNODE_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/stringoptionparameter.h"
#include "photon-global.h"

namespace photon {

class FixtureListParameter;

// Assigns a per-fixture time offset from where each fixture sits in a Falloff
// scene object (linear, radial or conical - see SceneFalloff).
//
//  - Bounded:   the falloff's own 0..1 amount at the fixture, including its
//               mirroring and hold/repeat/ping-pong past the end.
//  - Unbounded: the falloff only gives shape and direction. Its raw values are
//               stretched so the fixtures' own extremes land on 0 and 1.
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
