#ifndef PHOTON_PANMODELMODIFIER_H
#define PHOTON_PANMODELMODIFIER_H
#include "modelmodifier.h"

namespace photon {

class TransformComponent;
class AngleCapability;

class PanModelModifier : public ModelModifier
{


public:
    PanModelModifier(SceneObjectModel *model,  Entity *entity);


    void updateModel(const DMXMatrix &t_matrix, const double t_elapsed) override;

private:

    Axis m_axis;
    TransformComponent *m_transform;
    AngleCapability *m_capability;
};

} // namespace photon

#endif // PHOTON_PANMODELMODIFIER_H
