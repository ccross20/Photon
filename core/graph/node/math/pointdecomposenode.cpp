#include "pointdecomposenode.h"
#include "model/parameter/decimalparameter.h"
#include "graph/parameter/point2dparameter.h"

namespace photon {

const QByteArray PointDecomposeNode::PointInput = "pointInput";
const QByteArray PointDecomposeNode::XOutput = "xOutput";
const QByteArray PointDecomposeNode::YOutput = "yOutput";

class PointDecomposeNode::Impl
{
public:
    Point2DParameter *pointParam;
    keira::DecimalParameter *xParam;
    keira::DecimalParameter *yParam;
};

keira::NodeInformation PointDecomposeNode::info()
{
    keira::NodeInformation toReturn([](){return new PointDecomposeNode;});
    toReturn.name = "Point Decompose";
    toReturn.nodeId = "photon.math.point-decompose";
    toReturn.categories = {"Math"};

    return toReturn;
}

PointDecomposeNode::PointDecomposeNode() : keira::Node("photon.math.point-decompose"), m_impl(new Impl)
{
    setName("Point Decompose");
}

PointDecomposeNode::~PointDecomposeNode()
{
    delete m_impl;
}

void PointDecomposeNode::createParameters()
{
    m_impl->pointParam = new Point2DParameter(PointInput, "Point", QPointF());
    addParameter(m_impl->pointParam);

    m_impl->xParam = new keira::DecimalParameter(XOutput, "X", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->xParam);
    m_impl->yParam = new keira::DecimalParameter(YOutput, "Y", 0.0, keira::AllowMultipleOutput);
    addParameter(m_impl->yParam);
}

void PointDecomposeNode::evaluate(keira::EvaluationContext *) const
{
    const QPointF pt = m_impl->pointParam->value().toPointF();
    m_impl->xParam->setValue(pt.x());
    m_impl->yParam->setValue(pt.y());
}

} // namespace photon
