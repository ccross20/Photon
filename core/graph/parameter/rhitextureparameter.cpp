#include "rhitextureparameter.h"

namespace photon {

const QByteArray RhiTextureParameter::ParameterId = "rhiTexture";

RhiTextureParameter::RhiTextureParameter() : Parameter()
{
    // A texture handle can't be meaningfully value-compared for change (the pool
    // reuses pointers frame to frame), so always treat it as dirty — same as
    // TextureParameter.
    setTestValueForChange(false);
}

RhiTextureParameter::RhiTextureParameter(const QByteArray &t_id, const QString &t_name,
                                         RhiTextureData t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, t_default, connectionOptions)
{
    setTestValueForChange(false);
}

} // namespace photon
