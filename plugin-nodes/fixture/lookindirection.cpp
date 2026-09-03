#include <QtGui>

#include "lookindirection.h"
#include "beamangles.h"

namespace photon {

keira::NodeInformation LookInDirection::info()
{
    keira::NodeInformation toReturn([](){return new LookInDirection;});
    toReturn.name = "Look In Direction";
    toReturn.nodeId = "photon.plugin.node.look-in-direction";
    toReturn.categories = {"Fixture"};
    toReturn.graphs = QByteArrayList{"fixture"};

    return toReturn;
}

LookInDirection::LookInDirection() : keira::Node("photon.plugin.node.look-in-direction")
{

}

void LookInDirection::createParameters()
{
    m_matrixParam = new MatrixParameter("matrixInput","Matrix", QMatrix4x4{});
    addParameter(m_matrixParam);
    m_directionParam = new MatrixParameter("directionInput","Direction", QMatrix4x4{});
    addParameter(m_directionParam);

    m_invertPanParam = new keira::BooleanParameter("invertPan","Invert Pan", false);
    addParameter(m_invertPanParam);
    m_invertTiltParam = new keira::BooleanParameter("invertTilt","Invert Tilt", false);
    addParameter(m_invertTiltParam);

    m_tiltParam = new keira::DecimalParameter("tiltOutput","Tilt", 0.0, keira::AllowMultipleOutput);
    addParameter(m_tiltParam);
    m_panParam = new keira::DecimalParameter("panOutput","Pan", 0.0, keira::AllowMultipleOutput);
    addParameter(m_panParam);
}

void LookInDirection::evaluate(keira::EvaluationContext *t_context) const
{
    Q_UNUSED(t_context);

    const QMatrix4x4 frame = m_matrixParam->value().value<QMatrix4x4>();
    const QMatrix4x4 directionMatrix = m_directionParam->value().value<QMatrix4x4>();

    // A Direction helper draws its arrow along local +Y, so its world-space
    // facing is that axis carried through the matrix's rotation. mapVector drops
    // the translation - only the orientation matters here.
    const QVector3D worldDir = directionMatrix.mapVector(QVector3D{0.0f, 1.0f, 0.0f});

    // Express that facing in the fixture's local space. The fixture sits at its
    // local origin, so this is the direction the beam must point; again only the
    // frame's rotation is relevant, so map it as a vector.
    const QVector3D dir = frame.inverted().mapVector(worldDir);

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
