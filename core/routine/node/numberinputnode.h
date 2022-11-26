#ifndef PHOTON_NUMBERINPUTNODE_H
#define PHOTON_NUMBERINPUTNODE_H

#include "model/node.h"
#include "photon-global.h"
#include "sequence/channel.h"

namespace photon {

class PHOTONCORE_EXPORT NumberInputNode : public keira::Node
{
public:
    const static QByteArray Value;
    const static QByteArray DefaultValue;
    const static QByteArray Name;
    const static QByteArray Description;

    NumberInputNode();
    ~NumberInputNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    void setChannelIndex(uint);
    uint channelIndex() const;

    ChannelInfo channelInfo() const;

    static keira::NodeInformation info();


    void markDirty() override;


    void readFromJson(const QJsonObject &, keira::NodeLibrary *library) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_NUMBERINPUTNODE_H
