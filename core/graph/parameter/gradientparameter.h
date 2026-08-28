#ifndef PHOTON_GRADIENTPARAMETER_H
#define PHOTON_GRADIENTPARAMETER_H

#include "photon-global.h"
#include "model/parameter/parameter.h"
#include "util/gradient.h"

namespace photon {

class PHOTONCORE_EXPORT GradientParameter : public keira::Parameter
{
public:
    const static QByteArray ParameterId;

    GradientParameter();
    GradientParameter(const QByteArray &t_id, const QString &t_name, Gradient t_default = Gradient{},
                      int connectionOptions = keira::AllowSingleInput);

    QWidget *createWidget(keira::NodeEditor *) const override;
    void updateWidget(QWidget *) const override;
    QVariant updateValue(QWidget *) const override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;
};

} // namespace photon

#endif // PHOTON_GRADIENTPARAMETER_H
