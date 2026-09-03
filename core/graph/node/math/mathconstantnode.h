#ifndef PHOTON_MATHCONSTANTNODE_H
#define PHOTON_MATHCONSTANTNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Outputs a common math constant, chosen from a list.
class PHOTONCORE_EXPORT MathConstantNode : public keira::Node
{
public:
    enum Constant {
        Pi = 0,
        Tau = 1,
        HalfPi = 2,
        E = 3,
        Sqrt2 = 4,
        GoldenRatio = 5,
        Ln2 = 6,
        Ln10 = 7,
    };

    const static QByteArray ConstantInput;
    const static QByteArray Output;

    MathConstantNode();
    ~MathConstantNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_MATHCONSTANTNODE_H
