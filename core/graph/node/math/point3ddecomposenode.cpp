#include <QVector3D>
#include "point3ddecomposenode.h"
#include "model/parameter/decimalparameter.h"
#include "graph/parameter/vector3dparameter.h"

namespace photon {

const QByteArray Point3DDecomposeNode::PointInput = "pointInput";
const QByteArray Point3DDecomposeNode::XOutput = "xOutput";
const QByteArray Point3DDecomposeNode::YOutput = "yOutput";
const QByteArray Point3DDecomposeNode::ZOutput = "zOutput";

class Point3DDecomposeNode::Impl
{
public:
    Vector3DParameter *pointParam;
    keira::DecimalParameter *xParam;
    keira::DecimalParameter *yParam;
    keira::DecimalParameter *zParam;
};

keira::NodeInformation Point3DDecomposeNode::info()
{
    keira::NodeInformation toReturn([](){return new Point3DDecomposeNode;});
    toReturn.name = "Point 3D Decompose";
    toReturn.nodeId = "photon.math.point3d-decompose";
    toReturn.categories = {"Math"};

    return toReturn;
}

Point3DDecomposeNode::Point3DDecomposeNode() : keira::Node("photon.math.point3d-decompose"), m_impl(new Impl)
{
    setName("Point 3D Decompose");
}

Point3DDecomposeNode::~Point3DDecomposeNode()
{
    delete m_impl;
}

void Point3DDecomposeNode::createParameters()
{
    m_impl->pointParam = new Vector3DParameter(PointInput, "Point", QVector3D());
    addParameter(m_impl->pointParam);

    m_impl->xParam = new keira::DecimalParameter(XOutput, "X", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->xParam);
    m_impl->yParam = new keira::DecimalParameter(YOutput, "Y", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->yParam);
    m_impl->zParam = new keira::DecimalParameter(ZOutput, "Z", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->zParam);
}

void Point3DDecomposeNode::evaluate(keira::EvaluationContext *) const
{
    const QVector3D pt = m_impl->pointParam->value().value<QVector3D>();
    m_impl->xParam->setValue(pt.x());
    m_impl->yParam->setValue(pt.y());
    m_impl->zParam->setValue(pt.z());
}

} // namespace photon
