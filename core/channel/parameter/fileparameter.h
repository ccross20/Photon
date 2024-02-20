#ifndef PHOTON_FILEPARAMETER_H
#define PHOTON_FILEPARAMETER_H

#include "channelparameter.h"

namespace photon {

class PHOTONCORE_EXPORT FileParameter : public ChannelParameter
{
public:
    FileParameter();
    FileParameter(const QString &name, const QString &path);
    ~FileParameter();

    QString defaultValue() const;

    ChannelParameterView *createView() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FILEPARAMETER_H
