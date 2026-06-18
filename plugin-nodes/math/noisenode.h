#ifndef PHOTON_NOISENODE_H
#define PHOTON_NOISENODE_H

#include "model/node.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"
#include "model/parameter/optionparameter.h"
#include "graph/parameter/point2dparameter.h"
#include "util/noisegenerator.h"

namespace photon {

class NoiseNode : public keira::Node
{
public:

    NoiseNode();
    ~NoiseNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    Point2DParameter *scaleParam;
    keira::DecimalParameter *inputXParam;
    keira::DecimalParameter *inputYParam;
    Point2DParameter *inputFrequencyParam;
    keira::IntegerParameter *seedParam;
    keira::OptionParameter *noiseModeParam;
    keira::DecimalParameter *minParam;
    keira::DecimalParameter *maxParam;
    keira::DecimalParameter *outputParam;
    NoiseGenerator *m_noise;
};

} // namespace photon


#endif // PHOTON_NOISENODE_H
