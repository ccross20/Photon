#ifndef PHOTON_COLORCHANNELPARAMETER_H
#define PHOTON_COLORCHANNELPARAMETER_H
#include <QColor>
#include "channelparameter.h"


namespace photon {

class PHOTONCORE_EXPORT ColorChannelParameter : public ChannelParameter
{
public:
    ColorChannelParameter();
    ColorChannelParameter(const QString &name, QColor defaultValue = QColor{});
    ~ColorChannelParameter();

    QColor defaultValue() const;

    ChannelParameterView *createView() override;

    QVariant channelsToVariant(const QVector<double> &) const override;
    QVector<double> variantToChannels(const QVariant &) const override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;
private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_COLORCHANNELPARAMETER_H
