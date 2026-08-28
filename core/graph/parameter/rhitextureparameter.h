#ifndef PHOTON_RHITEXTUREPARAMETER_H
#define PHOTON_RHITEXTUREPARAMETER_H

#include <QSize>
#include <QVariant>
#include "photon-global.h"
#include "model/parameter/parameter.h"

class QRhiTexture;

namespace photon {

// The node-to-node currency of the GPU canvas graph: a lightweight, non-owning
// handle to a QRhiTexture (owned by RhiContext's pool). Nodes sample the input
// texture and render into a freshly acquired output texture, passing only this
// handle downstream — no pixel data ever leaves the GPU between nodes. This is
// the QRhi analog of the OpenGL-era TextureData/TextureParameter.
struct RhiTextureData
{
    RhiTextureData(QRhiTexture *texture = nullptr, QSize size = QSize())
        : texture(texture), size(size) {}

    QRhiTexture *texture = nullptr;
    QSize size;

    operator QVariant() const { return QVariant::fromValue(*this); }

    bool operator==(const RhiTextureData &o) const { return texture == o.texture && size == o.size; }
    bool operator!=(const RhiTextureData &o) const { return !(*this == o); }
};

class PHOTONCORE_EXPORT RhiTextureParameter : public keira::Parameter
{
public:
    const static QByteArray ParameterId;

    RhiTextureParameter();
    RhiTextureParameter(const QByteArray &t_id, const QString &t_name, RhiTextureData t_default,
                        int connectionOptions = keira::AllowSingleInput);
};

} // namespace photon

Q_DECLARE_METATYPE(photon::RhiTextureData);

#endif // PHOTON_RHITEXTUREPARAMETER_H
