#ifndef PHOTON_TILTMODELMODIFIER_H
#define PHOTON_TILTMODELMODIFIER_H
#include "modelmodifier.h"

namespace photon {

class TransformComponent;
class TiltCapability;

class TiltModelModifier : public ModelModifier
{


public:
    TiltModelModifier(FixtureModel *model,  Entity *entity);


    void updateModel(const DMXMatrix &t_matrix) override;

private:

    Axis m_axis;
    TransformComponent *m_transform;
    TiltCapability *m_capability;
};

} // namespace photon

#endif // PHOTON_TILTMODELMODIFIER_H
