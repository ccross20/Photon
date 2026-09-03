#ifndef PHOTON_MIRRORPANTILT_H
#define PHOTON_MIRRORPANTILT_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/optionparameter.h"
#include "graph/parameter/matrixparameter.h"

namespace photon {

// Mirrors a fixture's pan/tilt across a plane, but only for fixtures that sit on
// the mirrored side of it - fixtures on the other side pass their pan/tilt
// through untouched. That lets one aim graph drive a symmetric rig: the fixtures
// on the far side of the plane automatically get the mirror-image aim.
//
// The plane is a matrix (position + orientation); the Axis option picks which of
// its axes is the plane normal, and the sign picks which side is the mirrored
// one (the side the signed normal points toward). The fixture matrix (from a
// Fixture Info node) gives the fixture's world position - to test which side it
// is on - and its orientation, to move the beam between fixture-local pan/tilt
// and world space.
class MirrorPanTilt : public keira::Node
{
public:
    MirrorPanTilt();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    keira::DecimalParameter *m_panInParam;
    keira::DecimalParameter *m_tiltInParam;
    MatrixParameter *m_fixtureParam;
    MatrixParameter *m_planeParam;
    keira::OptionParameter *m_axisParam;
    keira::DecimalParameter *m_panOutParam;
    keira::DecimalParameter *m_tiltOutParam;
};

} // namespace photon

#endif // PHOTON_MIRRORPANTILT_H
