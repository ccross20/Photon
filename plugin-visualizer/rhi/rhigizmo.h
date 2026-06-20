#ifndef PHOTON_RHIGIZMO_H
#define PHOTON_RHIGIZMO_H

#include <QByteArray>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector3D>

namespace photon {

class SceneObject;
class RhiCamera;

// World-space transform gizmo (translate / rotate) acting on a SceneObject.
//
// CPU-only: it generates line geometry (pos+color) for the renderer to draw,
// picks handles against a world-space ray, and on drag writes the result back to
// the target's local position/rotation. Translation is exact (converted through
// the parent transform); rotation adds the swept world-axis angle to the matching
// Euler component (exact with no prior rotation, a usable approximation otherwise).
class RhiGizmo
{
public:
    enum Mode  { None, Translate, Rotate };
    enum Space { Global, Local };

    void setMode(Mode m)   { if (!m_dragging) m_mode  = m; }
    void setSpace(Space s) { if (!m_dragging) m_space = s; }
    Mode  mode()  const { return m_mode; }
    Space space() const { return m_space; }

    void setTarget(SceneObject *t) { if (!m_dragging) m_target = t; }
    SceneObject *target() const { return m_target; }

    bool isDragging() const { return m_dragging; }
    bool hasGizmo() const { return m_target && m_mode != None; }

    // Appends interleaved line vertices (vec3 position, vec3 color) in world space.
    void buildLines(const RhiCamera &cam, QByteArray &out) const;

    // Returns true if a handle was grabbed (caller should then suppress camera nav).
    bool beginDrag(const QVector3D &rayOrigin, const QVector3D &rayDir, const RhiCamera &cam);
    void updateDrag(const QVector3D &rayOrigin, const QVector3D &rayDir);
    void endDrag();

private:
    // Returns world-space direction of axis i, respecting global/local space.
    QVector3D axisDir(int i) const;
    float scaleFor(const RhiCamera &cam, const QVector3D &center) const;
    QMatrix4x4 parentGlobalMatrix() const;

    Mode  m_mode  = None;
    Space m_space = Global;
    SceneObject *m_target = nullptr;

    bool m_dragging  = false;
    int  m_activeAxis = -1;

    // Translate drag state.
    QVector3D  m_grabCenter;
    QVector3D  m_grabPoint;    // world-space hit point at drag begin (plane handles)
    QVector3D  m_dragAxisDir;  // axis dir (single-axis) or plane normal (plane handle)
    float      m_startParam = 0.0f;
    QMatrix4x4 m_parentInv;

    // Rotate drag state.
    QVector3D   m_center;
    QVector3D   m_planeU;
    QVector3D   m_planeW;
    float       m_startAngle = 0.0f;
    QQuaternion m_startQuat;
};

} // namespace photon

#endif // PHOTON_RHIGIZMO_H
