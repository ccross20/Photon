#ifndef PHOTON_DELAYNODE_H
#define PHOTON_DELAYNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT DelayNode : public keira::Node
{
public:
    const static QByteArray Input;
    const static QByteArray Output;
    const static QByteArray MaxDelay;

    DelayNode();
    ~DelayNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_DELAYNODE_H
