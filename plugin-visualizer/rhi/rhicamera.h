#ifndef PHOTON_RHICAMERA_H
#define PHOTON_RHICAMERA_H

#include <QMatrix4x4>
#include <QVector3D>
#include <QSize>
#include <QPointF>

namespace photon {

// Orbit camera producing separate view and projection matrices. The projection
// is a plain right-handed perspective with NO QRhi clip-space correction — the
// renderer applies QRhi::clipSpaceCorrMatrix() so this stays backend-agnostic.
class RhiCamera
{
public:
    RhiCamera();

    QMatrix4x4 viewMatrix() const;
    QMatrix4x4 projectionMatrix() const;
    QVector3D position() const;

    void setViewportSize(const QSize &size);
    QSize viewportSize() const { return m_viewport; }

    void orbit(float dxPixels, float dyPixels);
    void pan(float dxPixels, float dyPixels);
    void zoom(float steps);

    void setTarget(const QVector3D &target);
    void setDistance(float distance);

    // World-space ray through a pixel (top-left origin). For Milestone B picking.
    void rayAt(const QPointF &pixel, QVector3D &origin, QVector3D &direction) const;

private:
    QVector3D m_target;
    float     m_distance = 12.0f;
    float     m_yaw      = 35.0f;   // degrees about +Y
    float     m_pitch    = 25.0f;   // degrees above the XZ plane
    float     m_fovY     = 45.0f;
    float     m_near     = 0.05f;
    float     m_far      = 500.0f;
    QSize     m_viewport = QSize(1, 1);
};

} // namespace photon

#endif // PHOTON_RHICAMERA_H
