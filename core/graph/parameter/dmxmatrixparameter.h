#ifndef PHOTON_DMXMATRIXPARAMETER_H
#define PHOTON_DMXMATRIXPARAMETER_H
#include "photon-global.h"
#include "model/parameter/parameter.h"
#include "data/dmxmatrix.h"

namespace photon {

class PHOTONCORE_EXPORT DMXMatrixParameter : public keira::Parameter
{
public:
    const static QByteArray ParameterId;

    DMXMatrixParameter();
    DMXMatrixParameter(const QByteArray &t_id, const QString &t_name, DMXMatrix t_default, int connectionOptions = keira::AllowMultipleOutput | keira::AllowSingleInput);
    ~DMXMatrixParameter();


    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;
private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_DMXMATRIXPARAMETER_H
