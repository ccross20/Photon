#include "pointcomposenode.h"
#include "model/parameter/decimalparameter.h"
#include "graph/parameter/point2dparameter.h"

namespace photon {

const QByteArray PointComposeNode::XInput = "xInput";
const QByteArray PointComposeNode::YInput = "yInput";
const QByteArray PointComposeNode::PointOutput = "pointOutput";

class PointComposeNode::Impl
{
public:
    keira::DecimalParameter *xParam;
    keira::DecimalParameter *yParam;
    Point2DParameter *pointParam;
};

keira::NodeInformation PointComposeNode::info()
{
    keira::NodeInformation toReturn([](){return new PointComposeNode;});
    toReturn.name = "Point Compose";
    toReturn.nodeId = "photon.math.point-compose";
    toReturn.categories = {"Math"};

    return toReturn;
}

PointComposeNode::PointComposeNode() : keira::Node("photon.math.point-compose"), m_impl(new Impl)
{
    setName("Point Compose");
}

PointComposeNode::~PointComposeNode()
{
    delete m_impl;
}

void PointComposeNode::createParameters()
{
    m_impl->xParam = new keira::DecimalParameter(XInput, "X", 0.0);
    addParameter(m_impl->xParam);
    m_impl->yParam = new keira::DecimalParameter(YInput, "Y", 0.0);
    addParameter(m_impl->yParam);

    m_impl->pointParam = new Point2DParameter(PointOutput, "Point", QPointF(), keira::AllowMultipleOutput);
    addParameter(m_impl->pointParam);
}

void PointComposeNode::evaluate(keira::EvaluationContext *) const
{
    m_impl->pointParam->setValue(QPointF(m_impl->xParam->value().toDouble(),
                                         m_impl->yParam->value().toDouble()));
}

} // namespace photon
