#include "textureparameter.h"

namespace photon {

const QByteArray TextureParameter::ParameterId = "texture";

TextureParameter::TextureParameter() : Parameter()
{
    setTestValueForChange(false);
}

TextureParameter::TextureParameter(const QByteArray &t_id, const QString &t_name, TextureData t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, t_default, connectionOptions)
{
    setTestValueForChange(false);
}

} // namespace photon
