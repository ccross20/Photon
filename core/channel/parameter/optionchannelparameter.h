#ifndef PHOTON_OPTIONCHANNELPARAMETER_H
#define PHOTON_OPTIONCHANNELPARAMETER_H

#include "channelparameter.h"

namespace photon {

class PHOTONCORE_EXPORT OptionChannelParameter : public ChannelParameter
{
public:
    OptionChannelParameter();
    OptionChannelParameter(const QString &name, const QStringList &values, int defaultValue = 0);
    ~OptionChannelParameter();

    QStringList options() const;
    int defaultValue() const;
    ChannelParameterView *createView() override;


    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_OPTIONCHANNELPARAMETER_H
