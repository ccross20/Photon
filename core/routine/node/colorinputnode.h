#ifndef PHOTON_COLORINPUTNODE_H
#define PHOTON_COLORINPUTNODE_H

#include "model/node.h"
#include "photon-global.h"
#include "sequence/channel.h"

namespace photon {

class PHOTONCORE_EXPORT ColorInputNode : public keira::Node
{
public:
    const static QByteArray Value;
    const static QByteArray DefaultValue;
    const static QByteArray Name;
    const static QByteArray Description;

    ColorInputNode();
    ~ColorInputNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    void setChannelIndex(uint);
    uint channelIndex() const;

    ChannelInfo channelInfo() const;

    static keira::NodeInformation info();
    void setValue(const QByteArray &t_id, const QVariant &t_value) override;

    void markDirty() override;

    void readFromJson(const QJsonObject &, keira::NodeLibrary *library) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_COLORINPUTNODE_H
