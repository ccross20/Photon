#ifndef PHOTON_TEXTUREPARAMETER_H
#define PHOTON_TEXTUREPARAMETER_H

#include "photon-global.h"
#include "model/parameter/parameter.h"

namespace photon {

struct TextureData
{
    TextureData(uint handle = 0, QSize size = QSize{256,256}):handle(handle),size(size){}
    uint handle;
    QSize size;

    operator QVariant() const
    {
        return QVariant::fromValue(*this);
    }

    bool operator==(const TextureData &other) const { return handle == other.handle && size == other.size; }
    bool operator!=(const TextureData &other) const { return handle != other.handle || size != other.size; }
};


class PHOTONCORE_EXPORT TextureParameter : public keira::Parameter
{
public:
    const static QByteArray ParameterId;

    TextureParameter();
    TextureParameter(const QByteArray &t_id, const QString &t_name, TextureData t_default, int connectionOptions = keira::AllowMultipleOutput | keira::AllowSingleInput);
};

} // namespace photon

Q_DECLARE_METATYPE(photon::TextureData);

#endif // PHOTON_TEXTUREPARAMETER_H
