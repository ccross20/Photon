#ifndef WRITEDMXCHANNELNODE_H
#define WRITEDMXCHANNELNODE_H
#include "model/node.h"
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT WriteDMXChannelNode : public keira::Node
{
public:
    const static QByteArray InputDMX;
    const static QByteArray Fixture;
    const static QByteArray ChannelNumber;
    const static QByteArray ChannelValue;
    const static QByteArray OutputDMX;

    WriteDMXChannelNode();
    ~WriteDMXChannelNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;

    static keira::NodeInformation info();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // WRITEDMXCHANNELNODE_H
