#ifndef PHOTON_VECTOR3DPARAMETER_H
#define PHOTON_VECTOR3DPARAMETER_H

#include "photon-global.h"
#include "model/parameter/parameter.h"

namespace photon {

class PHOTONCORE_EXPORT Vector3DParameter : public keira::Parameter
{
public:
    const static QByteArray ParameterId;

    Vector3DParameter();
    Vector3DParameter(const QByteArray &t_id, const QString &t_name, QVector3D t_default, int connectionOptions = keira::AllowMultipleOutput | keira::AllowSingleInput);

    QWidget *createWidget(keira::NodeItem *) const override;
    void updateWidget(QWidget *) const override;
    QVariant updateValue(QWidget *) const override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;


};

} // namespace photon

#endif // PHOTON_VECTOR3DPARAMETER_H
