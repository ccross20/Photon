#include <QtGui>

#include "lookattarget.h"
#include "beamangles.h"

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
    m_matrixParam = new MatrixParameter("matrixInput","Matrix", QMatrix4x4{});
    addParameter(m_matrixParam);
    m_targetParam = new MatrixParameter("targetInput","Target Location", QMatrix4x4{});
    addParameter(m_targetParam);

    m_invertPanParam = new keira::BooleanParameter("invertPan","Invert Pan", false);
    addParameter(m_invertPanParam);
    m_invertTiltParam = new keira::BooleanParameter("invertTilt","Invert Tilt", false);
    addParameter(m_invertTiltParam);

    m_tiltParam = new keira::DecimalParameter("tiltOutput","Tilt", 0.0, keira::AllowMultipleOutput);
    addParameter(m_tiltParam);
    m_panParam = new keira::DecimalParameter("panOutput","Pan", 0.0, keira::AllowMultipleOutput);
    addParameter(m_panParam);

}

void LookAtTarget::evaluate(keira::EvaluationContext *t_context) const
{
    Q_UNUSED(t_context);

    const QMatrix4x4 frame = m_matrixParam->value().value<QMatrix4x4>();
    // Only the target's location matters for aiming, not its orientation -
    // take the translation out of its matrix.
    const QVector3D target = m_targetParam->value().value<QMatrix4x4>().map(QVector3D{});

    // Express the target in the fixture's local space. The fixture sits at the local
    // origin, so this local position is the direction the beam must point.
    const QVector3D dir = frame.inverted().map(target);

    const bool invertPan  = m_invertPanParam->value().toBool();
    const bool invertTilt = m_invertTiltParam->value().toBool();

    // Previous output, brought back into the non-inverted space the solver works in.
    double prevPan = m_panParam->value().toDouble();
    if(invertPan)
        prevPan = -prevPan;

    double pan, tilt;
    beamangles::panTiltFromDirection(dir, prevPan, invertPan, invertTilt, pan, tilt);

    m_panParam->setValue(pan);
    m_tiltParam->setValue(tilt);
}

} // namespace photon
