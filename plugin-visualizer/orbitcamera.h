#ifndef PHOTON_ORBITCAMERA_H
#define PHOTON_ORBITCAMERA_H
#include "camera.h"

namespace photon {

class OrbitCamera : public Camera
{
public:
    OrbitCamera();

    void setXRotation(double value);
    void setYRotation(double value);
    void setDistance(double value);

    double xRotation() const{return m_xRotation;}
    double yRotation() const{return m_yRotation;}
    double distance() const{return m_distance;}

private:
    void rebuild();

    double m_xRotation = 0;
    double m_yRotation = 0;
    double m_distance = 5;
};

} // namespace photon

#endif // PHOTON_ORBITCAMERA_H
