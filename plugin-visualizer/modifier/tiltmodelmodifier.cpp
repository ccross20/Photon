#include "tiltmodelmodifier.h"
#include "component/transformcomponent.h"
#include "fixture/fixture.h"
#include "fixture/capability/tiltcapability.h"

namespace photon {


void Lerper::setInitial(const QVector3D &value)
{
    m_target = value;
}
void Lerper::addValue(const QVector3D &value)
{
    m_target = value;
}
QVector3D Lerper::tick(double time)
{
    //qDebug() << m_target;
    return m_target;
}

TiltModelModifier::TiltModelModifier(SceneObjectModel *t_model,  Entity *t_entity) : ModelModifier(t_model, t_entity)
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

float lerp(float t_current, float t_target, float t_maxSpeed)
{
    if(abs(t_current - t_target) < t_maxSpeed)
        return t_target;

    if(t_target > t_current)
        return t_current + t_maxSpeed;
    else
        return t_current - t_maxSpeed;
}

void TiltModelModifier::preDraw(const double t_elapsed)
{
    m_transform->setRotationEuler(m_lerper.tick(t_elapsed));
}

void TiltModelModifier::updateModel(const DMXMatrix &t_matrix, const double t_elapsed)
{
    float percent = m_capability->getAnglePercent(t_matrix);

    float totalAngle = m_capability->angleEnd() - m_capability->angleStart();
    float halfAngle = totalAngle /2.0;
    float targetAngle = (totalAngle * percent) + halfAngle;

    /*
    switch (m_axis) {
    case Axis_X:
        m_transform->setRotationEuler(QVector3D{advancedLerp(m_transform->rotationEuler().x(), percent * 360.0f, 180.0*t_elapsed),0,0});
        break;
    case Axis_Y:
        m_transform->setRotationEuler(QVector3D{0,advancedLerp(m_transform->rotationEuler().y(), percent * 360.0f, 180.0*t_elapsed),0});
        break;
    case Axis_Z:
        m_transform->setRotationEuler(QVector3D{0,0,percent * 360.0f});
        break;
    }
    */

    switch (m_axis) {
    case Axis_X:
        m_lerper.addValue(QVector3D{targetAngle,0,0});
        break;
    case Axis_Y:
        m_lerper.addValue(QVector3D{0,targetAngle,0});
        break;
    case Axis_Z:
        m_lerper.addValue(QVector3D{0,0,targetAngle});
        break;
    }


}

} // namespace photon
