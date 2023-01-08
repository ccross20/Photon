#ifndef PHOTON_COLORPARAMETER_H
#define PHOTON_COLORPARAMETER_H

#include <QColor>
#include "photon-global.h"
#include "model/parameter/parameter.h"

namespace photon {

class PHOTONCORE_EXPORT ColorParameter : public keira::Parameter
{
public:

    const static QByteArray ParameterId;

    ColorParameter();
    ColorParameter(const QByteArray &t_id, const QString &t_name, QColor t_default, int connectionOptions = keira::AllowMultipleOutput | keira::AllowSingleInput);
    ~ColorParameter();

    QWidget *createWidget(keira::NodeItem *) const override;
    void updateWidget(QWidget *) const override;
    QVariant updateValue(QWidget *) const override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // PHOTON_COLORPARAMETER_H
