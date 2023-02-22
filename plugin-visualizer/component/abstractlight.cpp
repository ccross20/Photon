#include "abstractlight.h"
#include "entity.h"
#include "transformcomponent.h"

namespace photon {

AbstractLight::AbstractLight(LightType t_type):m_type(t_type)
{

}

AbstractLight::~AbstractLight()
{

}

void AbstractLight::setColor(QColor t_color)
{
    m_color = t_color;
}


void AbstractLight::setRange(double t_range)
{
    m_range = t_range;
}

void AbstractLight::setBrightness(double t_brightness)
{
    m_brightness = t_brightness;
}

QVector3D AbstractLight::globalDirection() const
{

    QVector3D v(0,0,-1);
    auto e = entity();
    if(!e)
        return v;

    auto transform = e->transform();
    if(!transform)
        return v;


    //auto mat = transform->globalMatrix();
    //return QVector3D(mat(0,0),mat(0,1),mat(0,2));

    return transform->globalMatrix().mapVector(v);
}

QVector3D AbstractLight::globalUpVector() const
{

    QVector3D v(0,1,0);
    auto e = entity();
    if(!e)
        return v;

    auto transform = e->transform();
    if(!transform)
        return v;


    //auto mat = transform->globalMatrix();
    //return QVector3D(mat(0,0),mat(0,1),mat(0,2));

    return transform->globalMatrix().mapVector(v);
}

QVector3D AbstractLight::globalPosition() const
{
    QVector3D v(0,0,0);
    auto e = entity();
    if(!e)
        return v;

    auto transform = e->transform();
    if(!transform)
        return v;

    return transform->globalPosition();
}

} // namespace photon
