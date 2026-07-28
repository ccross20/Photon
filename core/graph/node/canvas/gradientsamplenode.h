#ifndef PHOTON_GRADIENTSAMPLENODE_H
#define PHOTON_GRADIENTSAMPLENODE_H

#include "model/node.h"
#include "photon-global.h"

namespace keira {
class DecimalParameter;
}

namespace photon {

class GradientParameter;
class ColorParameter;

// Samples a gradient at a 0..1 position and outputs the resulting colour.
class PHOTONCORE_EXPORT GradientSampleNode : public keira::Node
{
public:
    const static QByteArray Input;
    const static QByteArray Position;
    const static QByteArray Output;

    GradientSampleNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    GradientParameter *m_input = nullptr;
    keira::DecimalParameter *m_position = nullptr;
    ColorParameter *m_output = nullptr;
};

} // namespace photon

#endif // PHOTON_GRADIENTSAMPLENODE_H
