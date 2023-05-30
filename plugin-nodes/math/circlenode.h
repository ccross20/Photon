#ifndef PHOTON_CIRCLENODE_H
#define PHOTON_CIRCLENODE_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

class CircleNode : public keira::Node
{
public:
    const static QByteArray AngleInput;
    const static QByteArray RadiusInput;
    const static QByteArray TiltOutput;
    const static QByteArray PanOutput;

    CircleNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    keira::DecimalParameter *angleParam;
    keira::DecimalParameter *radiusParam;
    keira::DecimalParameter *tiltParam;
    keira::DecimalParameter *panParam;
};

} // namespace photon


#endif // PHOTON_CIRCLENODE_H
