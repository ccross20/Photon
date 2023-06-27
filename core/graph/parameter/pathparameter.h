#ifndef PHOTON_PATHPARAMETER_H
#define PHOTON_PATHPARAMETER_H

#include <QPainterPath>
#include "photon-global.h"
#include "model/parameter/parameter.h"

namespace photon {

class PHOTONCORE_EXPORT PathParameter : public keira::Parameter
{
public:

    const static QByteArray ParameterId;

    PathParameter();
    PathParameter(const QByteArray &t_id, const QString &t_name, QPainterPath t_default, int connectionOptions = keira::AllowMultipleOutput | keira::AllowSingleInput);

};

} // namespace photon

#endif // PHOTON_PATHPARAMETER_H
