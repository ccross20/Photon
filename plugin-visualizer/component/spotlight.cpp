#include "spotlight.h"
#include "entity.h"
#include "transformcomponent.h"

namespace photon {

SpotLight::SpotLight() : AbstractLight(LightSpot)
{

}

SpotLight::~SpotLight()
{

}

QMatrix4x4 SpotLight::matrix() const
{
    auto e = entity();
    if(!e)
        return QMatrix4x4();

    auto transform = e->transform();
    if(!transform)
        return QMatrix4x4();

    QMatrix4x4 lightMatrix;
    lightMatrix.perspective(m_angle,1.0,0.01,60);


    auto pos = transform->globalPosition();


    lightMatrix.lookAt(pos, pos - globalDirection(), globalUpVector());



    return lightMatrix;

    //lightMatrix  = transform->globalMatrix() * lightMatrix;


    //return lightMatrix * transform->globalMatrix();
}

void SpotLight::setAngle(double t_angle)
{
    m_angle = t_angle;
}

void SpotLight::setHardness(double t_hardness)
{
    m_hardness = t_hardness;
}

void SpotLight::setCookie(const QImage &t_cookie)
{
    m_cookie = t_cookie;
}

void SpotLight::setGobo(const QImage &t_gobo)
{
    m_gobo = t_gobo;
}

} // namespace photon
