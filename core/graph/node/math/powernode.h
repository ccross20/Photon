#ifndef PHOTON_POWERNODE_H
#define PHOTON_POWERNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Raises Base to the Exponent power. Combinations with no real result (e.g. a
// negative base with a fractional exponent) output 0.
class PHOTONCORE_EXPORT PowerNode : public keira::Node
{
public:
    const static QByteArray Base;
    const static QByteArray Exponent;
    const static QByteArray Output;

    PowerNode();
    ~PowerNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_POWERNODE_H
