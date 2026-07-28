#ifndef PHOTON_GRADIENTMIXNODE_H
#define PHOTON_GRADIENTMIXNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace keira {
class DecimalParameter;
}

namespace photon {

class GradientParameter;

// Blends two gradients together by a 0..1 factor (0 = A, 1 = B).
class PHOTONCORE_EXPORT GradientMixNode : public keira::Node
{
public:
    const static QByteArray InputA;
    const static QByteArray InputB;
    const static QByteArray Factor;
    const static QByteArray Output;

    GradientMixNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    GradientParameter *m_inputA = nullptr;
    GradientParameter *m_inputB = nullptr;
    keira::DecimalParameter *m_factor = nullptr;
    GradientParameter *m_output = nullptr;
};

} // namespace photon

#endif // PHOTON_GRADIENTMIXNODE_H
