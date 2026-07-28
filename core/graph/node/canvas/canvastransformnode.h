#ifndef PHOTON_CANVASTRANSFORMNODE_H
#define PHOTON_CANVASTRANSFORMNODE_H

#include "basecanvasnode.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

// Canvas effect node: samples an input canvas under a 2D transform (translate,
// rotate, scale about the centre).
class PHOTONCORE_EXPORT CanvasTransformNode : public BaseCanvasNode
{
public:
    const static QByteArray Input;
    const static QByteArray TranslateX;
    const static QByteArray TranslateY;
    const static QByteArray Rotation;
    const static QByteArray Scale;
    const static QByteArray Output;

    CanvasTransformNode();

    void createParameters() override;
    static keira::NodeInformation info();

protected:
    QByteArray fragmentShaderName() const override { return "transform"; }
    quint32 uniformSize() const override { return 16; }   // 1 * vec4
    void writeUniforms(QByteArray &out, const QSize &size) const override;
    QVector<RhiTextureData> inputs() const override;

private:
    RhiTextureParameter *m_input = nullptr;
    keira::DecimalParameter *m_translateX = nullptr;
    keira::DecimalParameter *m_translateY = nullptr;
    keira::DecimalParameter *m_rotation = nullptr;
    keira::DecimalParameter *m_scale = nullptr;
};

} // namespace photon

#endif // PHOTON_CANVASTRANSFORMNODE_H
