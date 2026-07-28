#ifndef PHOTON_CANVASTILENODE_H
#define PHOTON_CANVASTILENODE_H

#include "basecanvasnode.h"
#include "model/parameter/booleanparameter.h"

namespace photon {

class Point2DParameter;

// Canvas effect node: tiles the input texture into a grid. Each axis can be a plain
// repeat, or mirrored (folded) so tiles meet seamlessly.
class PHOTONCORE_EXPORT CanvasTileNode : public BaseCanvasNode
{
public:
    const static QByteArray Input;
    const static QByteArray Tiles;
    const static QByteArray MirrorX;
    const static QByteArray MirrorY;
    const static QByteArray Output;

    CanvasTileNode();

    void createParameters() override;
    static keira::NodeInformation info();

protected:
    QByteArray fragmentShaderName() const override { return "tile"; }
    quint32 uniformSize() const override { return 16; }   // 1 * vec4
    void writeUniforms(QByteArray &out, const QSize &size) const override;
    QVector<RhiTextureData> inputs() const override;

private:
    RhiTextureParameter *m_input = nullptr;
    Point2DParameter *m_tiles = nullptr;
    keira::BooleanParameter *m_mirrorX = nullptr;
    keira::BooleanParameter *m_mirrorY = nullptr;
};

} // namespace photon

#endif // PHOTON_CANVASTILENODE_H
