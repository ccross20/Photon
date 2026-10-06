#include <QVector3D>
#include "point3dcomposenode.h"
#include "model/parameter/decimalparameter.h"
#include "graph/parameter/vector3dparameter.h"

namespace photon {

const QByteArray Point3DComposeNode::XInput = "xInput";
const QByteArray Point3DComposeNode::YInput = "yInput";
const QByteArray Point3DComposeNode::ZInput = "zInput";
const QByteArray Point3DComposeNode::PointOutput = "pointOutput";

class Point3DComposeNode::Impl
{
public:
    keira::DecimalParameter *xParam;
    keira::DecimalParameter *yParam;
    keira::DecimalParameter *zParam;
    Vector3DParameter *pointParam;
};

keira::NodeInformation Point3DComposeNode::info()
{
    keira::NodeInformation toReturn([](){return new Point3DComposeNode;});
    toReturn.name = "Point 3D Compose";
    toReturn.nodeId = "photon.math.point3d-compose";
    toReturn.categories = {"Math"};

    return toReturn;
}

Point3DComposeNode::Point3DComposeNode() : keira::Node("photon.math.point3d-compose"), m_impl(new Impl)
{
    setName("Point 3D Compose");
}

Point3DComposeNode::~Point3DComposeNode()
{
    delete m_impl;
}

void Point3DComposeNode::createParameters()
{
    m_impl->xParam = new keira::DecimalParameter(XInput, "X", 0.0);
    addParameter(m_impl->xParam);
    m_impl->yParam = new keira::DecimalParameter(YInput, "Y", 0.0);
    addParameter(m_impl->yParam);
    m_impl->zParam = new keira::DecimalParameter(ZInput, "Z", 0.0);
    addParameter(m_impl->zParam);

    m_impl->pointParam = new Vector3DParameter(PointOutput, "Point", QVector3D(), keira::AllowMultipleOutput);
    addParameter(m_impl->pointParam);
}

void Point3DComposeNode::evaluate(keira::EvaluationContext *) const
{
    m_impl->pointParam->setValue(QVector3D(m_impl->xParam->value().toFloat(),
                                           m_impl->yParam->value().toFloat(),
                                           m_impl->zParam->value().toFloat()));
}

} // namespace photon
