#ifndef PHOTON_CANVASPARAMETER_H
#define PHOTON_CANVASPARAMETER_H

#include <QImage>
#include "photon-global.h"
#include "model/parameter/parameter.h"

namespace photon {

class PHOTONCORE_EXPORT CanvasParameter : public keira::Parameter
{
public:

    const static QByteArray ParameterId;

    CanvasParameter();
    CanvasParameter(const QByteArray &t_id, const QString &t_name, QImage t_default, int connectionOptions = keira::AllowMultipleOutput | keira::AllowSingleInput);
    ~CanvasParameter();

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // PHOTON_CANVASPARAMETER_H
