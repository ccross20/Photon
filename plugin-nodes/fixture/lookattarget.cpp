#include <QMatrix4x4>
#include <QTransform>
#include <QtGui>
#include <math.h>

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

    m_tiltParam = new keira::DecimalParameter("tiltOutput","Tilt", 0.0);
    addParameter(m_tiltParam);
    m_panParam = new keira::DecimalParameter("panOutput","Pan", 0.0);
    addParameter(m_panParam);

}

double qCopysign(const double& x, const double y){
  if (y>=0.0) return fabs(x); else return -fabs(x); }

QQuaternion extractQuaternion(const QMatrix4x4& matrix)
{
    // Extract scale, shear and translation from matrix
    QMatrix4x4 transform = matrix.toTransform();
    qreal scaleX = transform.row(0).toVector3D().length();
    qreal scaleY = transform.row(1).toVector3D().length();
    qreal scaleZ = transform.row(2).toVector3D().length();
    QMatrix4x4 normalizeScale;
    normalizeScale.scale(1.0/scaleX, 1.0/scaleY, 1.0/scaleZ);
    QMatrix4x4 scaleShear = transform * normalizeScale;
    //QMatrix4x4 shear = normalizeScale.inverted() * scaleShear;

    // Extract rotation matrix from scale, shear and translation
    QMatrix4x4 rotation = scaleShear;
    rotation.setColumn(3, QVector4D(0, 0, 0, 1));
    //QVector3D translation = matrix.column(3).toVector3D();

    // Convert rotation matrix to quaternion
    qreal w = qSqrt(qMax(0.0, 1.0 + rotation.row(0).x() + rotation.row(1).y() + rotation.row(2).z())) / 2.0;
    qreal x = qSqrt(qMax(0.0, 1.0 + rotation.row(0).x() - rotation.row(1).y() - rotation.row(2).z())) / 2.0;
    qreal y = qSqrt(qMax(0.0, 1.0 - rotation.row(0).x() + rotation.row(1).y() - rotation.row(2).z())) / 2.0;
    qreal z = qSqrt(qMax(0.0, 1.0 - rotation.row(0).x() - rotation.row(1).y() + rotation.row(2).z())) / 2.0;
    x *= qCopysign(1.0, rotation.row(2).y() - rotation.row(1).z());
    y *= qCopysign(1.0, rotation.row(0).z() - rotation.row(2).x());
    z *= qCopysign(1.0, rotation.row(1).x() - rotation.row(0).y());

    return QQuaternion(w, x, y, z);
}

void LookAtTarget::evaluate(keira::EvaluationContext *t_context) const
{
    QMatrix4x4 matrix;
    matrix.translate(m_positionParam->value().value<QVector3D>());
    //matrix.rotate(QQuaternion::fromEulerAngles(m_rotationParam->value().value<QVector3D>()));

    //matrix = matrix.inverted();
    /*
    qDebug() << "Position" << m_positionParam->value().value<QVector3D>();
    qDebug() << "Target" << m_targetParam->value().value<QVector3D>();
    qDebug() << "Mapped Target" << matrix.map(m_targetParam->value().value<QVector3D>());
    */

    QMatrix4x4 basicMatrix;
    basicMatrix.lookAt(QVector3D{}, matrix.map(m_targetParam->value().value<QVector3D>()),matrix.mapVector(QVector3D{0,1,0}));

    //basicMatrix = basicMatrix.transposed();

    auto quat = extractQuaternion(basicMatrix);

    QVector3D rot = quat.toEulerAngles();
    /*
    rot.setX(qRadiansToDegrees(rot.x()));
    rot.setY(qRadiansToDegrees(rot.y()));
    rot.setZ(qRadiansToDegrees(rot.z()));
    */

    m_tiltParam->setValue(rot.x());

    qDebug() << "Look at pan" << rot.x();
    //m_panParam->setValue(rot.x());

    qDebug() << rot;
}

} // namespace photon
