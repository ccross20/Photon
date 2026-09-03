#ifndef PHOTON_LOOKINDIRECTION_H
#define PHOTON_LOOKINDIRECTION_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/booleanparameter.h"
#include "graph/parameter/matrixparameter.h"

namespace photon {

// Like Look At Target, but aimed by an orientation rather than a point: it
// outputs the Pan/Tilt that makes a fixture's beam face the same world
// direction a matrix points (typically a Direction helper object, whose arrow
// runs along its local +Y). Only the fixture matrix's rotation is used - a
// direction has no position, so distance doesn't matter.
class LookInDirection : public keira::Node
{
public:
    LookInDirection();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    MatrixParameter *m_matrixParam;
    MatrixParameter *m_directionParam;
    keira::DecimalParameter *m_tiltParam;
    keira::DecimalParameter *m_panParam;
    keira::BooleanParameter *m_invertPanParam;
    keira::BooleanParameter *m_invertTiltParam;
};

} // namespace photon

#endif // PHOTON_LOOKINDIRECTION_H
