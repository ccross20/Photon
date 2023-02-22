#ifndef PHOTON_FREECAMERA_H
#define PHOTON_FREECAMERA_H

#include "plugin-visualizer-global.h"
#include "camera.h"

namespace photon {

enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

// Default camera values
const float YAW         = -90.0f;
const float PITCH       =  0.0f;
const float SPEED       =  2.5f;
const float SENSITIVITY =  0.1f;


class FreeCamera : public Camera
{
public:

    FreeCamera(QVector3D position = QVector3D(0.0f, 0.0f, 0.0f), QVector3D up = QVector3D(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : m_front(QVector3D(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY)
    {
        m_position = position;
        m_worldUp = up;
        m_yaw = yaw;
        m_pitch = pitch;
        updateCameraVectors();
    }
    // constructor with scalar values
    FreeCamera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : m_front(QVector3D(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY)
    {
        m_position = QVector3D(posX, posY, posZ);
        m_worldUp = QVector3D(upX, upY, upZ);
        m_yaw = yaw;
        m_pitch = pitch;
        updateCameraVectors();
    }

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    QMatrix4x4 matrix() const override
    {
        QMatrix4x4 m;

        m.perspective(45,aspect(),0.01,1500);
        m.lookAt(m_position, m_position + m_front, m_up);
        return m;
    }

    QVector3D position() const override{
        return m_position;
    }

    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            m_position += m_front * velocity;
        if (direction == BACKWARD)
            m_position -= m_front * velocity;
        if (direction == LEFT)
            m_position -= m_right * velocity;
        if (direction == RIGHT)
            m_position += m_right * velocity;
        if (direction == UP)
            m_position -= m_up * velocity;
        if (direction == DOWN)
            m_position += m_up * velocity;
    }

    void StartOrbit(const QVector3D &centerPt = QVector3D{0,0,0})
    {
        m_orbitCenter = centerPt;
        m_orbitDistance = centerPt.distanceToPoint(m_position);
        m_orbitStartPosition = m_position;
        m_orbitStartMatrix = matrix();
        m_storedFront = m_front;
        m_storedUp = m_up;
        m_storedRight = m_right;
        //m_orbitMatrix.translate(m_orbitCenter);
    }

    void Orbit(float xoffset, float yoffset)
    {
        //m_orbitMatrix.rotate(QQuaternion::fromEulerAngles(yoffset,xoffset,0));

        m_orbitMatrix.setToIdentity();

        m_orbitMatrix.rotate(xoffset, QVector3D(0,1,0));
        m_orbitMatrix.rotate(std::min(std::max(yoffset,-89.f), 89.f), QVector3D(1,0,0));

        //m_orbitMatrix.lookAt(-QVector3D{x,y,z},QVector3D(0,0,0), QVector3D(0,1,0));

        //m_position = m_orbitMatrix.map(m_orbitStartPosition);

        auto mat = m_orbitMatrix;


        m_position = mat.map(m_orbitStartPosition);
        m_front = mat.map(m_storedFront);
        m_up = mat.map(m_storedUp);
        m_right = mat.map(m_storedRight);
        /*
        auto posCol = mat.column(3);
        m_position.setX(posCol.x());
        m_position.setY(posCol.y());
        m_position.setZ(posCol.z());
        */

    }

    void FreeRotate(float xoffset, float yoffset)
    {
        m_orbitMatrix.setToIdentity();

        m_orbitMatrix.rotate(xoffset, QVector3D(0,1,0));
        //m_orbitMatrix.rotate(yoffset, QVector3D(1,0,0));
        //m_position = m_orbitMatrix.map(m_orbitStartPosition);

        auto mat = m_orbitMatrix;


        m_front = mat.map(m_storedFront);
        m_up = mat.map(m_storedUp);
        m_right = mat.map(m_storedRight);
        /*
        auto posCol = mat.column(3);
        m_position.setX(posCol.x());
        m_position.setY(posCol.y());
        m_position.setZ(posCol.z());
        */

    }

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        m_yaw   += xoffset;
        m_pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (constrainPitch)
        {
            if (m_pitch > 89.0f)
                m_pitch = 89.0f;
            if (m_pitch < -89.0f)
                m_pitch = -89.0f;
        }

        // update Front, Right and Up Vectors using the updated Euler angles
        updateCameraVectors();
    }

    void ProcessMousePan(float xoffset, float yoffset)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;


            m_position -= m_right * xoffset;
            m_position += m_up * yoffset;
    }


    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void ProcessMouseScroll(float yoffset)
    {
        m_position += m_front * yoffset;

    }

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;


private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void updateCameraVectors()
    {
        // calculate the new Front vector
        QVector3D front;
        front.setX(cos(qDegreesToRadians(m_yaw)) * cos(qDegreesToRadians(m_pitch)));
        front.setY(sin(qDegreesToRadians(m_pitch)));
        front.setZ(sin(qDegreesToRadians(m_yaw)) * cos(qDegreesToRadians(m_pitch)));
        m_front = front.normalized();
        // also re-calculate the Right and Up vector

        m_right = QVector3D::crossProduct(m_front, m_worldUp).normalized();  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
        m_up    = QVector3D::crossProduct(m_right, m_front).normalized();
    }


    QVector3D m_position;
    QVector3D m_front;
    QVector3D m_up;
    QVector3D m_right;
    QVector3D m_worldUp;
    QVector3D m_storedUp;
    QVector3D m_storedFront;
    QVector3D m_storedRight;
    QVector3D m_orbitCenter;
    QMatrix4x4 m_orbitMatrix;
    QMatrix4x4 m_orbitStartMatrix;
    QVector3D m_orbitStartPosition;

    QPointF m_lastPosition;
    QVector3D m_startWorldPoint;
    // euler Angles
    float m_yaw;
    float m_pitch;
    float m_orbitDistance;
    // camera options
    float MovementSpeed;
    float MouseSensitivity;
};

} // namespace photon

#endif // PHOTON_FREECAMERA_H
