#include "fixtureparameter.h"

namespace photon {

const QByteArray FixtureParameter::ParameterId = "fixture";

FixtureParameter::FixtureParameter() : Parameter()
{

}

FixtureParameter::FixtureParameter(const QByteArray &t_id, const QString &t_name, QByteArray t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, t_default, connectionOptions)
{

}

} // namespace photon
