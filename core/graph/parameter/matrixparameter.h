#ifndef PHOTON_MATRIXPARAMETER_H
#define PHOTON_MATRIXPARAMETER_H

#include <QMatrix4x4>
#include "photon-global.h"
#include "model/parameter/parameter.h"

namespace photon {

// Carries a full 3D transform (position + rotation + scale, e.g. a scene
// object's global matrix) as a single value between nodes, rather than
// several separate position/rotation/scale ports - see MatrixDecomposeNode
// for pulling the parts back out when needed.
class PHOTONCORE_EXPORT MatrixParameter : public keira::Parameter
{
public:
    const static QByteArray ParameterId;

    MatrixParameter();
    MatrixParameter(const QByteArray &t_id, const QString &t_name, QMatrix4x4 t_default, int connectionOptions = keira::AllowSingleInput);
    ~MatrixParameter();

    QWidget *createWidget(keira::NodeEditor *) const override;
    void updateWidget(QWidget *) const override;
    QVariant updateValue(QWidget *) const override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_MATRIXPARAMETER_H
