#ifndef PHOTON_GRADIENTREVERSENODE_H
#define PHOTON_GRADIENTREVERSENODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

class GradientParameter;

// Mirrors a gradient's stop order (position -> 1 - position).
class PHOTONCORE_EXPORT GradientReverseNode : public keira::Node
{
public:
    const static QByteArray Input;
    const static QByteArray Output;

    GradientReverseNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    GradientParameter *m_input = nullptr;
    GradientParameter *m_output = nullptr;
};

} // namespace photon

#endif // PHOTON_GRADIENTREVERSENODE_H
