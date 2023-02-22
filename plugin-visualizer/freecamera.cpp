#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include "freecamera.h"

namespace photon {


void FreeCamera::keyPressEvent(QKeyEvent *event)
{

    switch (event->key()) {
    case Qt::Key_Left:
        ProcessKeyboard(LEFT, 1);
        break;
    case Qt::Key_Right:
        ProcessKeyboard(RIGHT, 1);
        break;
    case Qt::Key_Up:
        ProcessKeyboard(FORWARD, 1);
        break;
    case Qt::Key_Down:
        ProcessKeyboard(BACKWARD, 1);
        break;
    default:
        break;
    }

}

void FreeCamera::keyReleaseEvent(QKeyEvent *event)
{

}

void FreeCamera::mousePressEvent(QMouseEvent *event)
{
    Camera::mousePressEvent(event);
    m_lastPosition = event->pos();

    QVector3D pt(m_lastPosition.x(), m_lastPosition.y(), -1);
    auto mat = matrix();
    m_startWorldPoint =  mat.map(pt);

    if(event->buttons() & Qt::LeftButton)
        StartOrbit();
    if(event->buttons() & Qt::RightButton)
        StartOrbit(position());
}

void FreeCamera::mouseMoveEvent(QMouseEvent *event)
{
    Camera::mouseMoveEvent(event);

    if(event->buttons() & Qt::RightButton)
    {
        QPointF delta = event->pos() - m_lastPosition;

        FreeRotate(delta.x(), delta.y());
        //m_camera.ProcessMouseMovement(delta.x(), delta.y());

    }

    if(event->buttons() & Qt::LeftButton)
    {
        QPointF delta = event->pos() - m_lastPosition;

        Orbit(delta.x(), delta.y());
    }


    if(event->buttons() & Qt::MiddleButton)
    {
        QPointF delta = event->pos() - m_lastPosition;

        ProcessMousePan(delta.x(), delta.y());


        m_lastPosition = event->pos();
    }

    //m_lastPosition = event->pos();

}

void FreeCamera::mouseReleaseEvent(QMouseEvent *event)
{
    Camera::mouseReleaseEvent(event);
}

void FreeCamera::mouseDoubleClickEvent(QMouseEvent *event)
{
    Camera::mouseDoubleClickEvent(event);
}

void FreeCamera::wheelEvent(QWheelEvent *event)
{
    float delta = 1.05;
    if(event->angleDelta().y() > 0)
        ProcessMouseScroll(delta);
    else
        ProcessMouseScroll(-delta);

}

} // namespace photon
