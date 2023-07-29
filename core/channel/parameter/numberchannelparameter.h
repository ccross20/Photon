#ifndef PHOTON_NUMBERCHANNELPARAMETER_H
#define PHOTON_NUMBERCHANNELPARAMETER_H
#include "channelparameter.h"

namespace photon {

class PHOTONCORE_EXPORT NumberChannelParameter : public ChannelParameter
{
public:
    NumberChannelParameter();
    NumberChannelParameter(const QString &name, double defaultValue = 0.0, double min = std::numeric_limits<double>::lowest(), double max = std::numeric_limits<double>::max(), double increment = .01);
    ~NumberChannelParameter();

    double increment() const;
    double minimum() const;
    double maximum() const;
    double defaultValue() const;

    void setRange(double min, double max, double increment = .01);
    void setIncrement(double);
    void setMinimum(double);
    void setMaximum(double);
    ChannelParameterView *createView() override;


    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;
private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_NUMBERCHANNELPARAMETER_H
