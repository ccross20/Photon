#include "circlenode.h"

namespace photon {

const QByteArray CircleNode::AngleInput = "angle";
const QByteArray CircleNode::RadiusInput = "radius";
const QByteArray CircleNode::TiltOutput = "tilt";
const QByteArray CircleNode::PanOutput = "pan";


keira::NodeInformation CircleNode::info()
{
    keira::NodeInformation toReturn([](){return new CircleNode;});
    toReturn.name = "Circle";
    toReturn.nodeId = "photon.math.circle";
    toReturn.categories = {"Math"};

    return toReturn;
}

CircleNode::CircleNode() : keira::Node("photon.math.circle")
{
    setName("Circle");
}

void CircleNode::createParameters()
{
    angleParam = new keira::DecimalParameter(AngleInput,"Angle", 0.0);
    addParameter(angleParam);
    radiusParam = new keira::DecimalParameter(RadiusInput,"Radius", 2.0);
    addParameter(radiusParam);

    tiltParam = new keira::DecimalParameter(TiltOutput,"Tilt", 0.0, keira::AllowMultipleOutput);
    addParameter(tiltParam);
    panParam = new keira::DecimalParameter(PanOutput,"Pan", 0.0, keira::AllowMultipleOutput);
    addParameter(panParam);
}

void CircleNode::evaluate(keira::EvaluationContext *t_context) const
{
    tiltParam->setValue(std::cos(angleParam->value().toDouble()) * radiusParam->value().toDouble());
    panParam->setValue(std::sin(angleParam->value().toDouble()) * radiusParam->value().toDouble());

}

} // namespace photon
