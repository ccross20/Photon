#ifndef PHOTON_GRADIENTREMAPNODE_H
#define PHOTON_GRADIENTREMAPNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace keira {
class DecimalParameter;
class BooleanParameter;
}

namespace photon {

class GradientParameter;

// Shifts and scales a gradient's stop positions. In Repeat mode the gradient is
// treated as periodic so animating Offset scrolls/cycles the colours; in Clamp
// mode transformed positions are bounded to [0,1].
class PHOTONCORE_EXPORT GradientRemapNode : public keira::Node
{
public:
    const static QByteArray Input;
    const static QByteArray Offset;
    const static QByteArray Scale;
    const static QByteArray Repeat;
    const static QByteArray Output;

    GradientRemapNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    GradientParameter *m_input = nullptr;
    keira::DecimalParameter *m_offset = nullptr;
    keira::DecimalParameter *m_scale = nullptr;
    keira::BooleanParameter *m_repeat = nullptr;
    GradientParameter *m_output = nullptr;
};

} // namespace photon

#endif // PHOTON_GRADIENTREMAPNODE_H
