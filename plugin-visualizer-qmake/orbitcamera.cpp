#include "orbitcamera.h"
#include <math.h>


namespace photon {

OrbitCamera::OrbitCamera()
{
    rebuild();
}

void OrbitCamera::rebuild()
{
    double x = std::cos((m_xRotation*M_PI)/180.0) * m_distance;
    double y = std::sin((m_yRotation*M_PI)/180.0) * m_distance;
    double z = std::sin((m_xRotation*M_PI)/180.0) * m_distance;

    setPosition(QVector3D(x,y,z));

}

void OrbitCamera::setXRotation(double value)
{
    m_xRotation = value;
    rebuild();
}

void OrbitCamera::setYRotation(double value)
{
    m_yRotation = fmin(fmax(value,-45),45);
    rebuild();
}

void OrbitCamera::setDistance(double value)
{
    m_distance = value;
    rebuild();
}


} // namespace photon
