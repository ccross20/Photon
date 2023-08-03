#ifndef PHOTON_BOOLCHANNELPARAMETER_H
#define PHOTON_BOOLCHANNELPARAMETER_H

#include "channelparameter.h"

namespace photon {

class PHOTONCORE_EXPORT BoolChannelParameter : public ChannelParameter
{
public:
    BoolChannelParameter();
    BoolChannelParameter(const QString &name, bool defaultValue = true);
    ~BoolChannelParameter();

    bool defaultValue() const;

    ChannelParameterView *createView() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;
private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_BOOLCHANNELPARAMETER_H
