#ifndef PHOTON_INVERTNODE_H
#define PHOTON_INVERTNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

// Boolean NOT: outputs true when the input is false and vice versa.
class PHOTONCORE_EXPORT InvertNode : public keira::Node
{
public:
    const static QByteArray Input;
    const static QByteArray Output;

    InvertNode();
    ~InvertNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_INVERTNODE_H
