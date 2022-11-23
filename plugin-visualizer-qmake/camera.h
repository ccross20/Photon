#ifndef PHOTON_CAMERA_H
#define PHOTON_CAMERA_H

#include "plugin-visualizer-global.h"

namespace photon {

class Camera
{
public:
    Camera();
    virtual ~Camera();

    void setPosition(const QVector3D &position);
    void setEulerRotation(const QVector3D &rotation);
    void setAspect(double aspect);
    double aspect() const;
    QVector3D eulerRotation() const;
    virtual QVector3D position() const;

    virtual QMatrix4x4 matrix() const{return m_matrix;}

private:
    void rebuildMatrix();

    QVector3D m_position;
    QVector3D m_rotation;
    QMatrix4x4 m_matrix;
    double m_aspect = 1.0;
};

} // namespace photon

#endif // PHOTON_CAMERA_H
