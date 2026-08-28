#ifndef PHOTON_LOOKATTARGET_H
#define PHOTON_LOOKATTARGET_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/booleanparameter.h"
#include "graph/parameter/matrixparameter.h"

namespace photon {

class LookAtTarget : public keira::Node
{
public:
    LookAtTarget();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    MatrixParameter *m_matrixParam;
    MatrixParameter *m_targetParam;
    keira::DecimalParameter *m_tiltParam;
    keira::DecimalParameter *m_panParam;
    keira::BooleanParameter *m_invertPanParam;
    keira::BooleanParameter *m_invertTiltParam;
};

} // namespace photon

#endif // PHOTON_LOOKATTARGET_H
