#ifndef PHOTON_CHANNELPARAMETER_H
#define PHOTON_CHANNELPARAMETER_H

#include "photon-global.h"
#include "sequence/channel.h"

namespace photon {

class PHOTONCORE_EXPORT ChannelParameter
{
public:
    enum ChannelParameterType
    {
        ChannelParameterNumber,
        ChannelParameterColor,
        ChannelParameterPoint,
        ChannelParameterText
    };


    ChannelParameter(const QString &name,ChannelParameterType type = ChannelParameterNumber, int channelCount = 1);
    virtual ~ChannelParameter();

    QString name() const;
    QStringList channelNames() const;
    void setChannelNames(const QStringList &);
    QByteArray uniqueId() const;
    int channels();
    QVariant value() const;
    void setValue(const QVariant &);
    bool isReadOnly() const;
    void setIsReadOnly(bool);
    ChannelParameterType type() const;
    virtual ChannelInfo channelInfo() const;

    virtual ChannelParameterView *createView() = 0;
    virtual QVariant channelsToVariant(const QVector<double> &) const;
    virtual QVector<double> variantToChannels(const QVariant &) const;

    virtual void readFromJson(const QJsonObject &);
    virtual void writeToJson(QJsonObject &) const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CHANNELPARAMETER_H
