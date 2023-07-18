#ifndef PHOTON_SACNTRANSMITNODE_H
#define PHOTON_SACNTRANSMITNODE_H

#include "model/node.h"
#include "photon-global.h"

namespace photon {

class SACNTransmitNode : public keira::Node
{
public:


    const static QByteArray InputDMX;
    const static QByteArray Network;

    SACNTransmitNode();
    ~SACNTransmitNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon


#endif // PHOTON_SACNTRANSMITNODE_H
