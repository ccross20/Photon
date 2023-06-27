#include "panmodelmodifier.h"
#include "component/transformcomponent.h"
#include "fixture/fixture.h"
#include "fixture/capability/pancapability.h"

namespace photon {

PanModelModifier::PanModelModifier(SceneObjectModel *t_model,  Entity *t_entity) : ModelModifier(t_model, t_entity)
{
    auto pans = fixture()->findCapability(Capability_Pan);

    if(pans.length() > 0)
    {
        m_capability = static_cast<PanCapability*>(pans[0]);
        m_transform = entity()->findComponent<TransformComponent*>();
    }

    if(t_entity->name().endsWith("_x"))
        m_axis = Axis_X;
    else if(t_entity->name().endsWith("_y"))
        m_axis = Axis_Y;
    else if(t_entity->name().endsWith("_z"))
        m_axis = Axis_Z;
}

void PanModelModifier::updateModel(const DMXMatrix &t_matrix, const double t_elapsed)
{
    float percent = m_capability->getAnglePercent(t_matrix);
    float range = m_capability->angleEnd() - m_capability->angleStart();
    float angle = (percent * range) - (range/2.0f);

    switch (m_axis) {
    case Axis_X:
        m_transform->setRotationEuler(QVector3D{angle,0,0});
        break;
    case Axis_Y:
        m_transform->setRotationEuler(QVector3D{0,angle,0});
        break;
    case Axis_Z:
        m_transform->setRotationEuler(QVector3D{0,0,angle});
        break;
    }


}

} // namespace photon
