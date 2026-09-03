#include <QtGui>

#include "mirrorpantilt.h"
#include "beamangles.h"

namespace photon {

keira::NodeInformation MirrorPanTilt::info()
{
    keira::NodeInformation toReturn([](){return new MirrorPanTilt;});
    toReturn.name = "Mirror Pan/Tilt";
    toReturn.nodeId = "photon.plugin.node.mirror-pan-tilt";
    toReturn.categories = {"Fixture"};
    toReturn.graphs = QByteArrayList{"fixture"};

    return toReturn;
}

MirrorPanTilt::MirrorPanTilt() : keira::Node("photon.plugin.node.mirror-pan-tilt")
{

}

void MirrorPanTilt::createParameters()
{
    m_panInParam = new keira::DecimalParameter("panInput","Pan", 0.0);
    addParameter(m_panInParam);
    m_tiltInParam = new keira::DecimalParameter("tiltInput","Tilt", 0.0);
    addParameter(m_tiltInParam);

    m_fixtureParam = new MatrixParameter("fixtureInput","Fixture Matrix", QMatrix4x4{});
    addParameter(m_fixtureParam);

    m_planeParam = new MatrixParameter("planeInput","Plane", QMatrix4x4{});
    addParameter(m_planeParam);

    // Which of the plane matrix's axes is the mirror normal, and which side the
    // normal points toward - fixtures on that side get mirrored, the rest pass
    // through. Appended, never reordered (the stored value is the option index).
    m_axisParam = new keira::OptionParameter("axis","Axis", {
        "+X",
        "-X",
        "+Y",
        "-Y",
        "+Z",
        "-Z",
    }, 0);
    addParameter(m_axisParam);

    m_panOutParam = new keira::DecimalParameter("panOutput","Pan", 0.0, keira::AllowMultipleOutput);
    addParameter(m_panOutParam);
    m_tiltOutParam = new keira::DecimalParameter("tiltOutput","Tilt", 0.0, keira::AllowMultipleOutput);
    addParameter(m_tiltOutParam);
}

void MirrorPanTilt::evaluate(keira::EvaluationContext *t_context) const
{
    Q_UNUSED(t_context);

    const double panIn  = m_panInParam->value().toDouble();
    const double tiltIn = m_tiltInParam->value().toDouble();

    const QMatrix4x4 fixtureFrame = m_fixtureParam->value().value<QMatrix4x4>();
    const QMatrix4x4 plane = m_planeParam->value().value<QMatrix4x4>();

    const int option = qBound(0, m_axisParam->value().toInt(), 5);
    const int axisIndex = option / 2;                 // 0=X, 1=Y, 2=Z
    const float axisSign = (option % 2 == 0) ? 1.0f : -1.0f;

    // Plane normal in world space, oriented by the option's sign so it points at
    // the side that should be mirrored. Reflection itself is sign-independent;
    // the sign only decides which side of the plane the gate below mirrors.
    const QVector4D axisCol = plane.column(axisIndex);
    QVector3D normal = axisSign * QVector3D(axisCol.x(), axisCol.y(), axisCol.z());
    if(normal.lengthSquared() < 1e-8f)
    {
        // Degenerate plane matrix - nothing to mirror about.
        m_panOutParam->setValue(panIn);
        m_tiltOutParam->setValue(tiltIn);
        return;
    }
    normal.normalize();

    // Signed distance from the fixture to the plane. Only fixtures on the side
    // the normal points toward get mirrored; everything else passes through so
    // the same graph can drive both halves of a symmetric rig.
    const QVector3D fixturePos = fixtureFrame.column(3).toVector3D();
    const QVector3D planePos = plane.column(3).toVector3D();
    if(QVector3D::dotProduct(fixturePos - planePos, normal) < 0.0f)
    {
        m_panOutParam->setValue(panIn);
        m_tiltOutParam->setValue(tiltIn);
        return;
    }

    // Fixture-local beam direction -> world, reflect across the plane, back to
    // fixture-local, then solve for the mirrored pan/tilt. Only the frame's
    // rotation matters for a direction, so map it as a vector.
    const QVector3D worldDir = fixtureFrame.mapVector(beamangles::directionFromPanTilt(panIn, tiltIn));
    const QVector3D mirrored = worldDir - 2.0f * QVector3D::dotProduct(worldDir, normal) * normal;
    const QVector3D localDir = fixtureFrame.inverted().mapVector(mirrored);

    double panOut, tiltOut;
    beamangles::panTiltFromDirection(localDir, m_panOutParam->value().toDouble(),
                                     false, false, panOut, tiltOut);

    m_panOutParam->setValue(panOut);
    m_tiltOutParam->setValue(tiltOut);
}

} // namespace photon
