#ifndef PHOTON_RANDOMCOLORNODE_H
#define PHOTON_RANDOMCOLORNODE_H

#include "model/node.h"
#include "model/parameter/integerparameter.h"
#include "graph/parameter/colorparameter.h"

namespace photon {

// Outputs a random hue at full saturation and brightness. The Seed fully
// determines the colour, so the same seed always gives the same colour.
class RandomColorNode : public keira::Node
{
public:
    RandomColorNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    keira::IntegerParameter *m_seedParam;
    ColorParameter *m_outputParam;
};

} // namespace photon

#endif // PHOTON_RANDOMCOLORNODE_H
