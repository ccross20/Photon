#include "camera.h"

namespace photon {

Camera::Camera()
{

}

Camera::~Camera()
{

}

void Camera::setPosition(const QVector3D &t_position)
{
    m_position = t_position;
    rebuildMatrix();
}

void Camera::setEulerRotation(const QVector3D &t_rotation)
{
    m_rotation = t_rotation;
    rebuildMatrix();
}

QVector3D Camera::eulerRotation() const
{
    return m_rotation;
}

QVector3D Camera::position() const
{
    return m_position;
}

void Camera::setAspect(double aspect)
{
    m_aspect = aspect;
    rebuildMatrix();
}

double Camera::aspect() const
{
    return m_aspect;
}

void Camera::rebuildMatrix()
{
    m_matrix.setToIdentity();

    m_matrix.perspective(45,m_aspect,.01,100);
    //m_matrix.translate(-m_position);
    m_matrix.lookAt(-m_position,QVector3D(0,0,0), QVector3D(0,1,0));
}
void Camera::keyPressEvent(QKeyEvent *event)
{

}

void Camera::keyReleaseEvent(QKeyEvent *event)
{

}

void Camera::mousePressEvent(QMouseEvent *event)
{

}

void Camera::mouseMoveEvent(QMouseEvent *event)
{

}

void Camera::mouseReleaseEvent(QMouseEvent *event)
{

}

void Camera::mouseDoubleClickEvent(QMouseEvent *event)
{

}

void Camera::wheelEvent(QWheelEvent *event)
{

}


} // namespace photon
