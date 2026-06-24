#include <QMatrix4x4>
#include <QTransform>
#include <QtGui>
#include <cmath>

#include "lookattarget.h"
#include "routine/routineevaluationcontext.h"

namespace photon {

keira::NodeInformation LookAtTarget::info()
{
    keira::NodeInformation toReturn([](){return new LookAtTarget;});
    toReturn.name = "Look At Target";
    toReturn.nodeId = "photon.plugin.node.look-at-target";

    return toReturn;
}

LookAtTarget::LookAtTarget() : keira::Node("photon.plugin.node.look-at-target")
{

}

void LookAtTarget::createParameters()
{
    m_positionParam = new Vector3DParameter("positionInput","Position", QVector3D{});
    addParameter(m_positionParam);
    m_rotationParam = new Vector3DParameter("rotationInput","Rotation", QVector3D{});
    addParameter(m_rotationParam);
    m_targetParam = new Vector3DParameter("targetInput","Target Position", QVector3D{});
    addParameter(m_targetParam);

    m_invertPanParam = new keira::BooleanParameter("invertPan","Invert Pan", false);
    addParameter(m_invertPanParam);
    m_invertTiltParam = new keira::BooleanParameter("invertTilt","Invert Tilt", false);
    addParameter(m_invertTiltParam);

    m_tiltParam = new keira::DecimalParameter("tiltOutput","Tilt", 0.0);
    addParameter(m_tiltParam);
    m_panParam = new keira::DecimalParameter("panOutput","Pan", 0.0);
    addParameter(m_panParam);

}

void LookAtTarget::evaluate(keira::EvaluationContext *t_context) const
{
    Q_UNUSED(t_context);

    const QVector3D pos    = m_positionParam->value().value<QVector3D>();
    const QVector3D rot    = m_rotationParam->value().value<QVector3D>();
    const QVector3D target = m_targetParam->value().value<QVector3D>();

    // Fixture world frame (matches Fixture::globalMatrix: translate, then rotate).
    QMatrix4x4 frame;
    frame.translate(pos);
    frame.rotate(QQuaternion::fromEulerAngles(rot));

    // Express the target in the fixture's local space. The fixture sits at the local
    // origin, so this local position is the direction the beam must point.
    QVector3D dir = frame.inverted().map(target);
    if(dir.lengthSquared() < 1e-8f)
    {
        m_panParam->setValue(0.0);
        m_tiltParam->setValue(0.0);
        return;
    }
    dir.normalize();

    // The beam aims along local -Y, rotated by pan (about +Y) then tilt (about +X) —
    // the same order the visualiser applies them. That gives the local direction
    //   ( -sin(pan)*sin(tilt), -cos(tilt), -cos(pan)*sin(tilt) ).
    // Inverting gives tilt from the Y component and pan from X/Z. Angles are centred
    // (0,0 = beam straight down the fixture's -Y), matching the centred-degrees mode
    // of the Set Fixture Pan/Tilt nodes.
    const double tiltA = qRadiansToDegrees(std::acos(qBound(-1.0f, -dir.y(), 1.0f)));
    const double horiz = std::sqrt(double(dir.x()) * dir.x() + double(dir.z()) * dir.z());

    // Continuity: the same beam direction is given by both (pan, tilt) and
    // (pan+180, -tilt). Near the pan axis (target almost directly below/above) pan is
    // ill-defined and the naive solution flips 180°, spinning the head as the target
    // passes underneath. Pick the solution closest to the previous output so the head
    // tilts smoothly through instead of flipping, and hold pan at the singularity.
    const bool invertPan  = m_invertPanParam->value().toBool();
    const bool invertTilt = m_invertTiltParam->value().toBool();

    // Previous output, brought back into the non-inverted space the solver works in.
    double prevPan = m_panParam->value().toDouble();
    if(invertPan)
        prevPan = -prevPan;

    double pan, tilt;
    if(horiz < 1e-4)
    {
        pan  = prevPan;                                  // pan undefined here
        tilt = (dir.y() <= 0.0f) ? 0.0 : 180.0;          // straight down / up
    }
    else
    {
        double panA = qRadiansToDegrees(std::atan2(-dir.x(), -dir.z()));
        double panB = panA + 180.0;
        if(panB > 180.0) panB -= 360.0;

        auto panDist = [](double a, double b) {
            return std::abs(std::fmod(a - b + 540.0, 360.0) - 180.0);
        };
        if(panDist(panB, prevPan) < panDist(panA, prevPan))
        { pan = panB; tilt = -tiltA; }                   // flipped solution, same aim
        else
        { pan = panA; tilt = tiltA; }
    }

    m_panParam->setValue(invertPan ? -pan : pan);
    m_tiltParam->setValue(invertTilt ? -tilt : tilt);
}

} // namespace photon
