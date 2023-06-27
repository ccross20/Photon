#include "pathparameter.h"

namespace photon {

const QByteArray PathParameter::ParameterId = "path";

PathParameter::PathParameter() : Parameter()
{

}

PathParameter::PathParameter(const QByteArray &t_id, const QString &t_name, QPainterPath t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, QVariant::fromValue(t_default), connectionOptions)
{

}


} // namespace photon
