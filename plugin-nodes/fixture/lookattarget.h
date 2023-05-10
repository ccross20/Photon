#ifndef PHOTON_LOOKATTARGET_H
#define PHOTON_LOOKATTARGET_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "graph/parameter/vector3dparameter.h"

namespace photon {

class LookAtTarget : public keira::Node
{
public:
    LookAtTarget();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

private:
    Vector3DParameter *m_positionParam;
    Vector3DParameter *m_rotationParam;
    Vector3DParameter *m_targetParam;
    keira::DecimalParameter *m_tiltParam;
    keira::DecimalParameter *m_panParam;
};

} // namespace photon

#endif // PHOTON_LOOKATTARGET_H
