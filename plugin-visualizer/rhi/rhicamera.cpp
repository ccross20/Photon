#include "rhicamera.h"
#include <QtMath>

namespace photon {

RhiCamera::RhiCamera() {}

QVector3D RhiCamera::position() const
{
    const float yaw   = qDegreesToRadians(m_yaw);
    const float pitch = qDegreesToRadians(m_pitch);
    const QVector3D dir(std::cos(pitch) * std::sin(yaw),
                        std::sin(pitch),
                        std::cos(pitch) * std::cos(yaw));
    return m_target + dir * m_distance;
}

QMatrix4x4 RhiCamera::viewMatrix() const
{
    QMatrix4x4 view;
    view.lookAt(position(), m_target, QVector3D(0, 1, 0));
    return view;
}

QMatrix4x4 RhiCamera::projectionMatrix() const
{
    QMatrix4x4 proj;
    const float aspect = m_viewport.height() > 0
        ? float(m_viewport.width()) / float(m_viewport.height())
        : 1.0f;
    proj.perspective(m_fovY, aspect, m_near, m_far);
    return proj;
}

void RhiCamera::setViewportSize(const QSize &size)
{
    m_viewport = size.isEmpty() ? QSize(1, 1) : size;
}

void RhiCamera::orbit(float dxPixels, float dyPixels)
{
    m_yaw   -= dxPixels * 0.4f;
    m_pitch += dyPixels * 0.4f;
    m_pitch = qBound(-89.0f, m_pitch, 89.0f);
}

void RhiCamera::pan(float dxPixels, float dyPixels)
{
    // World units per pixel at the focus distance.
    const float worldPerPixel = m_viewport.height() > 0
        ? (2.0f * m_distance * std::tan(qDegreesToRadians(m_fovY * 0.5f))) / m_viewport.height()
        : 0.0f;

    const QMatrix4x4 view = viewMatrix();
    // Rows of the rotation part give camera basis vectors in world space.
    const QVector3D right(view(0, 0), view(0, 1), view(0, 2));
    const QVector3D up   (view(1, 0), view(1, 1), view(1, 2));

    m_target += (-dxPixels * worldPerPixel) * right;
    m_target += ( dyPixels * worldPerPixel) * up;
}

void RhiCamera::zoom(float steps)
{
    m_distance *= std::pow(0.9f, steps);
    m_distance = qBound(0.2f, m_distance, 400.0f);
}

void RhiCamera::setTarget(const QVector3D &target)
{
    m_target = target;
}

void RhiCamera::setDistance(float distance)
{
    m_distance = qBound(0.2f, distance, 400.0f);
}

void RhiCamera::rayAt(const QPointF &pixel, QVector3D &origin, QVector3D &direction) const
{
    const float x = 2.0f * float(pixel.x()) / float(m_viewport.width())  - 1.0f;
    const float y = 1.0f - 2.0f * float(pixel.y()) / float(m_viewport.height());

    const QMatrix4x4 invVP = (projectionMatrix() * viewMatrix()).inverted();
    const QVector3D nearPt = invVP.map(QVector3D(x, y, -1.0f)); // near plane
    const QVector3D farPt  = invVP.map(QVector3D(x, y,  1.0f)); // far plane

    origin = nearPt;
    direction = (farPt - nearPt).normalized();
}

} // namespace photon
