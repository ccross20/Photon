#include <QtMath>
#include "rhigizmo.h"
#include "rhicamera.h"
#include "scene/sceneobject.h"

namespace photon {

namespace {

constexpr float kPi = 3.14159265358979323846f;

const QVector3D kAxis[3]  = { {1,0,0}, {0,1,0}, {0,0,1} };
const QVector3D kColor[3] = { {0.90f,0.25f,0.25f}, {0.30f,0.85f,0.30f}, {0.35f,0.45f,0.95f} };
const QVector3D kActive   = { 1.0f, 0.85f, 0.20f };
// Plane handle colors (CMY — complement of the RGB axis colors).
// Index p is the plane whose normal is axis p (i.e. the YZ, XZ, XY planes).
const QVector3D kPlaneColor[3] = {
    {0.20f, 0.80f, 0.80f},   // cyan    – YZ plane (moves in Y+Z)
    {0.80f, 0.20f, 0.80f},   // magenta – XZ plane (moves in X+Z)
    {0.80f, 0.80f, 0.20f},   // yellow  – XY plane (moves in X+Y)
};
// Square spans [pLo, pHi] along each of the two in-plane axes (fraction of scale).
constexpr float kPlaneLo = 0.20f;
constexpr float kPlaneHi = 0.45f;

QVector3D perp1(const QVector3D &a)
{
    const QVector3D ref = std::abs(a.y()) > 0.99f ? QVector3D(1, 0, 0) : QVector3D(0, 1, 0);
    return QVector3D::crossProduct(ref, a).normalized();
}
QVector3D perp2(const QVector3D &a)
{
    return QVector3D::crossProduct(a, perp1(a)).normalized();
}

void pushVertex(QByteArray &b, const QVector3D &p, const QVector3D &c)
{
    const float f[6] = { p.x(), p.y(), p.z(), c.x(), c.y(), c.z() };
    b.append(reinterpret_cast<const char *>(f), sizeof(f));
}
void pushSeg(QByteArray &b, const QVector3D &a, const QVector3D &c, const QVector3D &col)
{
    pushVertex(b, a, col);
    pushVertex(b, c, col);
}

// Closest distance between the ray (O,D) and the axis line (A0 + s*Adir), plus the
// axis parameter s of the closest point.
void closestAxisParam(const QVector3D &O, const QVector3D &D,
                      const QVector3D &A0, const QVector3D &Adir,
                      float &dist, float &s)
{
    const QVector3D w0 = A0 - O;
    const float b = QVector3D::dotProduct(Adir, D);
    const float d = QVector3D::dotProduct(Adir, w0);
    const float e = QVector3D::dotProduct(D, w0);
    const float denom = 1.0f - b * b;
    if (std::abs(denom) < 1e-6f) {       // nearly parallel
        s = 0.0f;
        dist = (w0 - D * e).length();
        return;
    }
    s = (b * e - d) / denom;
    const float t = (e - b * d) / denom;
    dist = ((A0 + Adir * s) - (O + D * t)).length();
}

bool rayPlane(const QVector3D &O, const QVector3D &D,
              const QVector3D &C, const QVector3D &N, float &t)
{
    const float denom = QVector3D::dotProduct(D, N);
    if (std::abs(denom) < 1e-6f)
        return false;
    t = QVector3D::dotProduct(C - O, N) / denom;
    return t > 0.0f;
}

} // namespace

QVector3D RhiGizmo::axisDir(int i) const
{
    if (m_space == Global || !m_target)
        return kAxis[i];
    // Extract column i from the upper-left 3x3 of the global matrix — that gives
    // the object's local axis i expressed in world space.
    const QMatrix4x4 gm = m_target->globalMatrix();
    return QVector3D(gm(0, i), gm(1, i), gm(2, i)).normalized();
}

float RhiGizmo::scaleFor(const RhiCamera &cam, const QVector3D &center) const
{
    return qMax(0.05f, float((cam.position() - center).length()) * 0.18f);
}

QMatrix4x4 RhiGizmo::parentGlobalMatrix() const
{
    SceneObject *p = m_target ? m_target->parentSceneObject() : nullptr;
    return p ? p->globalMatrix() : QMatrix4x4();
}

void RhiGizmo::buildLines(const RhiCamera &cam, QByteArray &out) const
{
    out.clear();
    if (!hasGizmo())
        return;

    const QVector3D center = m_target->globalPosition();
    const float scale = scaleFor(cam, center);

    if (m_mode == Translate) {
        // Axis arrows
        for (int i = 0; i < 3; ++i) {
            const QVector3D col = (m_activeAxis == i) ? kActive : kColor[i];
            const QVector3D ax  = axisDir(i);
            const QVector3D tip = center + ax * scale;
            pushSeg(out, center, tip, col);

            // Arrowhead: four short lines from the tip.
            const QVector3D u = perp1(ax);
            const QVector3D w = perp2(ax);
            const QVector3D back = tip - ax * (scale * 0.18f);
            const float hw = scale * 0.06f;
            pushSeg(out, tip, back + u * hw, col);
            pushSeg(out, tip, back - u * hw, col);
            pushSeg(out, tip, back + w * hw, col);
            pushSeg(out, tip, back - w * hw, col);
        }

        // Plane handle squares — one per axis pair, positioned at [pLo, pHi]
        // along the two in-plane axes.
        for (int p = 0; p < 3; ++p) {
            const int a1 = (p + 1) % 3;
            const int a2 = (p + 2) % 3;
            const QVector3D col = (m_activeAxis == 3 + p) ? kActive : kPlaneColor[p];
            const QVector3D d1 = axisDir(a1) * scale;
            const QVector3D d2 = axisDir(a2) * scale;
            const QVector3D c0 = center + d1 * kPlaneLo + d2 * kPlaneLo;
            const QVector3D c1 = center + d1 * kPlaneHi + d2 * kPlaneLo;
            const QVector3D c2 = center + d1 * kPlaneHi + d2 * kPlaneHi;
            const QVector3D c3 = center + d1 * kPlaneLo + d2 * kPlaneHi;
            pushSeg(out, c0, c1, col);
            pushSeg(out, c1, c2, col);
            pushSeg(out, c2, c3, col);
            pushSeg(out, c3, c0, col);
        }
    } else { // Rotate rings
        const int seg = 64;
        for (int i = 0; i < 3; ++i) {
            const QVector3D col = (m_activeAxis == i) ? kActive : kColor[i];
            const QVector3D ax  = axisDir(i);
            const QVector3D u = perp1(ax);
            const QVector3D w = perp2(ax);
            QVector3D prev = center + u * scale;
            for (int s = 1; s <= seg; ++s) {
                const float a = (2.0f * kPi) * float(s) / float(seg);
                const QVector3D p = center + (std::cos(a) * u + std::sin(a) * w) * scale;
                pushSeg(out, prev, p, col);
                prev = p;
            }
        }
    }
}

bool RhiGizmo::beginDrag(const QVector3D &O, const QVector3D &D, const RhiCamera &cam)
{
    if (!hasGizmo())
        return false;

    const QVector3D center = m_target->globalPosition();
    const float scale = scaleFor(cam, center);

    if (m_mode == Translate) {
        // 1. Try axis arrows first.
        float bestDist = scale * 0.12f;
        int bestAxis = -1;
        for (int i = 0; i < 3; ++i) {
            float dist, s;
            closestAxisParam(O, D, center, axisDir(i), dist, s);
            if (dist < bestDist && s > -scale * 0.1f && s < scale * 1.2f) {
                bestDist = dist;
                bestAxis = i;
            }
        }
        if (bestAxis >= 0) {
            m_activeAxis  = bestAxis;
            m_grabCenter  = center;
            m_dragAxisDir = axisDir(bestAxis);
            float dist;
            closestAxisParam(O, D, m_grabCenter, m_dragAxisDir, dist, m_startParam);
            m_parentInv = parentGlobalMatrix().inverted();
            m_dragging  = true;
            return true;
        }

        // 2. Try plane handle squares.
        const float pLo = scale * kPlaneLo;
        const float pHi = scale * kPlaneHi;
        for (int p = 0; p < 3; ++p) {
            const int a1 = (p + 1) % 3;
            const int a2 = (p + 2) % 3;
            const QVector3D perpDir = axisDir(p);
            float t;
            if (!rayPlane(O, D, center, perpDir, t) || t <= 0.0f)
                continue;
            const QVector3D hit = O + D * t;
            const QVector3D rel = hit - center;
            const float c1 = QVector3D::dotProduct(rel, axisDir(a1));
            const float c2 = QVector3D::dotProduct(rel, axisDir(a2));
            if (c1 >= pLo && c1 <= pHi && c2 >= pLo && c2 <= pHi) {
                m_activeAxis  = 3 + p;
                m_grabCenter  = center;
                m_grabPoint   = hit;
                m_dragAxisDir = perpDir; // plane normal — reused for rayPlane in update
                m_parentInv   = parentGlobalMatrix().inverted();
                m_dragging    = true;
                return true;
            }
        }

        return false;
    }

    // Rotate
    float bestErr = scale * 0.10f;
    int bestAxis = -1;
    for (int i = 0; i < 3; ++i) {
        float t;
        if (rayPlane(O, D, center, axisDir(i), t)) {
            const float r = ((O + D * t) - center).length();
            const float err = std::abs(r - scale);
            if (err < bestErr) {
                bestErr = err;
                bestAxis = i;
            }
        }
    }
    if (bestAxis < 0)
        return false;

    m_activeAxis  = bestAxis;
    m_center      = center;
    m_dragAxisDir = axisDir(bestAxis);
    m_planeU = perp1(m_dragAxisDir);
    m_planeW = perp2(m_dragAxisDir);
    float t;
    rayPlane(O, D, center, m_dragAxisDir, t);
    const QVector3D v = (O + D * t) - center;
    m_startAngle = std::atan2(QVector3D::dotProduct(v, m_planeW),
                              QVector3D::dotProduct(v, m_planeU));
    m_startQuat  = QQuaternion::fromEulerAngles(m_target->rotation());
    m_dragging = true;
    return true;
}

void RhiGizmo::updateDrag(const QVector3D &O, const QVector3D &D)
{
    if (!m_dragging || !m_target || m_activeAxis < 0)
        return;

    if (m_mode == Translate) {
        if (m_activeAxis >= 3) {
            // Plane handle: slide along the captured plane.
            float t;
            if (!rayPlane(O, D, m_grabCenter, m_dragAxisDir, t))
                return;
            const QVector3D newWorld = m_grabCenter + (O + D * t) - m_grabPoint;
            m_target->setPosition(m_parentInv.map(newWorld));
        } else {
            // Single-axis arrow.
            float dist, s;
            closestAxisParam(O, D, m_grabCenter, m_dragAxisDir, dist, s);
            const QVector3D newWorld = m_grabCenter + m_dragAxisDir * (s - m_startParam);
            m_target->setPosition(m_parentInv.map(newWorld));
        }
    } else {
        float t;
        if (!rayPlane(O, D, m_center, m_dragAxisDir, t))
            return;
        const QVector3D v = (O + D * t) - m_center;
        const float ang = std::atan2(QVector3D::dotProduct(v, m_planeW),
                                     QVector3D::dotProduct(v, m_planeU));
        const float deltaDegs = float(qRadiansToDegrees(ang - m_startAngle));
        // Use kAxis[i] (the pure local basis vector) for the delta quaternion in both
        // modes — the difference is whether we pre- or post-multiply relative to the
        // start orientation.
        //   Global: rotate in world frame  → newQ = deltaQ * startQ
        //   Local:  rotate in object frame → newQ = startQ * deltaQ
        const QQuaternion deltaQ =
            QQuaternion::fromAxisAndAngle(kAxis[m_activeAxis], deltaDegs);
        const QQuaternion newQuat = (m_space == Local)
            ? m_startQuat * deltaQ
            : deltaQ * m_startQuat;
        m_target->setRotation(newQuat.toEulerAngles());
    }
}

void RhiGizmo::endDrag()
{
    m_dragging = false;
    m_activeAxis = -1;
}

} // namespace photon
