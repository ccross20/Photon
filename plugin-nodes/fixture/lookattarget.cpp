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

void LookAtTarget::evaluate(keira::EvaluationContext *t_context) const
{
    QMatrix4x4 matrix;
    matrix.translate(m_positionParam->value().value<QVector3D>());
    matrix.rotate(QQuaternion::fromEulerAngles(m_rotationParam->value().value<QVector3D>()));


    auto panRad = matrix.inverted().map(m_targetParam->value().value<QVector3D>());
    float panDeg = qRadiansToDegrees(atan2(panRad.x(),panRad.z()));

    //qDebug() << panDeg;

    if(panDeg > 90)
        panDeg -= 180;
    else if(panDeg < -90)
        panDeg += 180;

    m_panParam->setValue(panDeg);

    QMatrix4x4 tiltMat;
    tiltMat.translate(0,.4,0);
    tiltMat.rotate(QQuaternion::fromEulerAngles(QVector3D{0,panDeg,90}));

    matrix = matrix * tiltMat;

    auto tiltRad = matrix.inverted().map(m_targetParam->value().value<QVector3D>());
    float tiltDeg = qRadiansToDegrees(atan2(tiltRad.z(),tiltRad.x()));
    m_tiltParam->setValue(tiltDeg);

}

} // namespace photon
