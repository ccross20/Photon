#ifndef PHOTON_PANMODELMODIFIER_H
#define PHOTON_PANMODELMODIFIER_H
#include "modelmodifier.h"

namespace photon {

class TransformComponent;
class PanCapability;

class PanModelModifier : public ModelModifier
{


public:
    PanModelModifier(FixtureModel *model,  Entity *entity);


    void updateModel(const DMXMatrix &t_matrix) override;

private:

    Axis m_axis;
    TransformComponent *m_transform;
    PanCapability *m_capability;
};

} // namespace photon

#endif // PHOTON_PANMODELMODIFIER_H
