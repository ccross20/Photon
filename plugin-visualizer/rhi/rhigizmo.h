#ifndef PHOTON_RHIGIZMO_H
#define PHOTON_RHIGIZMO_H

#include <QByteArray>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector>
#include <QVector3D>

namespace photon {

class SceneObject;
class RhiCamera;

// World-space transform gizmo (translate / rotate) acting on one or more
// SceneObjects.
//
// CPU-only: it generates line geometry (pos+color) for the renderer to draw,
// picks handles against a world-space ray, and on drag writes the result back to
// the targets' local position/rotation. The gizmo itself is drawn at the
// centroid of all targets' world positions.
//
// Translate applies the same world-space delta to every target (each mapped
// through its own parent transform, so targets with different parents still
// move correctly together). Rotate orbits every target's position around that
// shared centroid and applies the same delta rotation to each target's own
// orientation - a single target's position is unaffected (it's already at the
// centroid), so this doesn't change existing single-select behavior.
// Translation is exact (converted through each target's own parent transform);
// rotation adds the swept world-axis angle to the matching Euler component -
// exact with no prior rotation, a usable approximation otherwise.
//
// Scale (zone size handles) stays single-target - it operates on the last
// target if that's a SceneZone, matching prior single-select behavior.
class RhiGizmo
{
public:
    enum Mode  { None, Translate, Rotate, Scale };
    enum Space { Global, Local };

    void setMode(Mode m)   { if (!m_dragging) m_mode  = m; }
    void setSpace(Space s) { if (!m_dragging) m_space = s; }
    Mode  mode()  const { return m_mode; }
    Space space() const { return m_space; }

    void setTargets(const QVector<SceneObject *> &targets) { if (!m_dragging) m_targets = targets; }
    const QVector<SceneObject *> &targets() const { return m_targets; }

    bool isDragging() const { return m_dragging; }
    bool hasGizmo() const { return !m_targets.isEmpty() && m_mode != None; }

    // Appends interleaved line vertices (vec3 position, vec3 color) in world space.
    void buildLines(const RhiCamera &cam, QByteArray &out) const;

    // Returns true if a handle was grabbed (caller should then suppress camera nav).
    bool beginDrag(const QVector3D &rayOrigin, const QVector3D &rayDir, const RhiCamera &cam);
    void updateDrag(const QVector3D &rayOrigin, const QVector3D &rayDir);
    void endDrag();
    // Swaps the objects a drag in progress is moving, keeping the grabbed
    // handle - used by Cmd-drag to carry on dragging freshly made copies
    // instead of the originals. The new targets must sit where the old ones
    // did when the drag began (copies made before the drag moved anything).
    void retargetDrag(const QVector<SceneObject *> &targets);

private:
    // Returns world-space direction of axis i, respecting global/local space.
    // Local space with multiple targets uses the last ("active") target's frame.
    QVector3D axisDir(int i) const;
    float scaleFor(const RhiCamera &cam, const QVector3D &center) const;
    // Centroid of all targets' world positions - where the gizmo is drawn/grabbed.
    QVector3D pivotPosition() const;
    // Captures each target's world position (+ rotation, if includeRotation) and
    // parent-inverse matrix at drag start, ready for updateDrag() to apply a
    // shared delta to every target.
    void captureTargetState(bool includeRotation);

    Mode  m_mode  = None;
    Space m_space = Global;
    QVector<SceneObject *> m_targets;

    bool m_dragging  = false;
    int  m_activeAxis = -1;

    // Translate/rotate drag state, shared across all targets.
    QVector3D  m_grabCenter;  // pivot at drag start (translate ref point / rotate orbit center)
    QVector3D  m_grabPoint;   // world-space hit point at drag begin (plane handles)
    QVector3D  m_dragAxisDir; // axis dir (single-axis) or plane normal (plane handle)
    float      m_startParam = 0.0f;

    // Per-target state captured at drag begin (translate and rotate).
    QVector<QVector3D>   m_startWorldPos;
    QVector<QMatrix4x4>  m_parentInvs;
    QVector<QQuaternion> m_startQuats;   // rotate mode only

    // Scale drag state (zones, single target).
    QVector3D   m_startSize;

    // Rotate drag state.
    QVector3D   m_planeU;
    QVector3D   m_planeW;
    float       m_startAngle = 0.0f;
};

} // namespace photon

#endif // PHOTON_RHIGIZMO_H
