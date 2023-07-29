#ifndef PHOTON_POINT2CHANNELPARAMETER_H
#define PHOTON_POINT2CHANNELPARAMETER_H
#include "channelparameter.h"


namespace photon {

class PHOTONCORE_EXPORT Point2ChannelParameter : public ChannelParameter
{
public:
    Point2ChannelParameter();
    Point2ChannelParameter(const QString &name, QPointF defaultValue = QPointF{}, QPointF min = QPointF{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()},
                           QPointF max = QPointF{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()}, double increment = .01);
    ~Point2ChannelParameter();

    double increment() const;
    QPointF minimum() const;
    QPointF maximum() const;
    QPointF defaultValue() const;

    void setRange(QPointF min, QPointF max, double increment = .01);
    void setIncrement(double);
    void setMinimum(QPointF);
    void setMaximum(QPointF);
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

#endif // PHOTON_POINT2CHANNELPARAMETER_H
