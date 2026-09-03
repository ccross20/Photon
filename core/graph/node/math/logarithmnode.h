#ifndef PHOTON_LOGARITHMNODE_H
#define PHOTON_LOGARITHMNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Logarithm of the input in the selected base. A non-positive input outputs 0
// rather than -inf / NaN.
class PHOTONCORE_EXPORT LogarithmNode : public keira::Node
{
public:
    enum Mode { ModeNatural = 0, ModeBase10 = 1, ModeBase2 = 2 };

    const static QByteArray Input;
    const static QByteArray ModeInput;
    const static QByteArray Output;

    LogarithmNode();
    ~LogarithmNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_LOGARITHMNODE_H
