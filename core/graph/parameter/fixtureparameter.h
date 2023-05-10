#ifndef PHOTON_FIXTUREPARAMETER_H
#define PHOTON_FIXTUREPARAMETER_H

#include "photon-global.h"
#include "model/parameter/parameter.h"

namespace photon {

class PHOTONCORE_EXPORT FixtureParameter : public keira::Parameter
{
public:
    const static QByteArray ParameterId;

    FixtureParameter();
    FixtureParameter(const QByteArray &t_id, const QString &t_name, QByteArray t_default, int connectionOptions = keira::AllowMultipleOutput | keira::AllowSingleInput);
};

} // namespace photon

#endif // PHOTON_FIXTUREPARAMETER_H
