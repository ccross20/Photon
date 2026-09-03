#ifndef PHOTON_MATHFUNCTIONNODE_H
#define PHOTON_MATHFUNCTIONNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// A grab-bag of single-input math operations, selected by Mode. Absolute and
// Square Root are their own nodes; this covers the less-reached-for ones.
class PHOTONCORE_EXPORT MathFunctionNode : public keira::Node
{
public:
    enum Mode {
        ModeSquare = 0,
        ModeSign = 1,
        ModeNegate = 2,
        ModeReciprocal = 3,
        ModeFractional = 4,
        ModeExponential = 5,
    };

    const static QByteArray Input;
    const static QByteArray ModeInput;
    const static QByteArray Output;

    MathFunctionNode();
    ~MathFunctionNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_MATHFUNCTIONNODE_H
