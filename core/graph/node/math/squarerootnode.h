#ifndef PHOTON_SQUAREROOTNODE_H
#define PHOTON_SQUAREROOTNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Outputs the square root of the input. A negative input outputs 0 rather than
// a NaN that would poison everything downstream.
class PHOTONCORE_EXPORT SquareRootNode : public keira::Node
{
public:
    const static QByteArray Input;
    const static QByteArray Output;

    SquareRootNode();
    ~SquareRootNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SQUAREROOTNODE_H
