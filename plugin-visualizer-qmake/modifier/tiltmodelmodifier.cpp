#include "tiltmodelmodifier.h"
#include "component/transformcomponent.h"
#include "fixture/fixture.h"
#include "fixture/capability/tiltcapability.h"

namespace photon {

TiltModelModifier::TiltModelModifier(FixtureModel *t_model,  Entity *t_entity) : ModelModifier(t_model, t_entity)
{
    auto capabilities = fixture()->findCapability(Capability_Tilt);

    if(capabilities.length() > 0)
    {
        m_capability = static_cast<TiltCapability*>(capabilities[0]);
        m_transform = entity()->findComponent<TransformComponent*>();
    }

    if(t_entity->name().endsWith("_x"))
        m_axis = Axis_X;
    else if(t_entity->name().endsWith("_y"))
        m_axis = Axis_Y;
    else if(t_entity->name().endsWith("_z"))
        m_axis = Axis_Z;
}

void TiltModelModifier::updateModel(const DMXMatrix &t_matrix)
{
    float percent = m_capability->getAnglePercent(t_matrix);
    switch (m_axis) {
    case Axis_X:
        m_transform->setRotationEuler(QVector3D{percent * 360.0f,0,0});
        break;
    case Axis_Y:
        m_transform->setRotationEuler(QVector3D{0,percent * 360.0f,0});
        break;
    case Axis_Z:
        m_transform->setRotationEuler(QVector3D{0,0,percent * 360.0f});
        break;
    }


}

} // namespace photon
