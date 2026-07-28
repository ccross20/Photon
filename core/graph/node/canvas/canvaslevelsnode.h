#ifndef PHOTON_CANVASLEVELSNODE_H
#define PHOTON_CANVASLEVELSNODE_H

#include "basecanvasnode.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

// Canvas effect node: Photoshop-style Levels. Remaps input tones between an input
// black/white point with a gamma (midtone) curve, then onto an output black/white
// range. Applied to RGB.
class PHOTONCORE_EXPORT CanvasLevelsNode : public BaseCanvasNode
{
public:
    const static QByteArray Input;
    const static QByteArray InputBlack;
    const static QByteArray InputWhite;
    const static QByteArray Gamma;
    const static QByteArray OutputBlack;
    const static QByteArray OutputWhite;
    const static QByteArray Output;

    CanvasLevelsNode();

    void createParameters() override;
    static keira::NodeInformation info();

protected:
    QByteArray fragmentShaderName() const override { return "levels"; }
    quint32 uniformSize() const override { return 32; }   // 2 * vec4
    void writeUniforms(QByteArray &out, const QSize &size) const override;
    QVector<RhiTextureData> inputs() const override;

private:
    RhiTextureParameter *m_input = nullptr;
    keira::DecimalParameter *m_inputBlack = nullptr;
    keira::DecimalParameter *m_inputWhite = nullptr;
    keira::DecimalParameter *m_gamma = nullptr;
    keira::DecimalParameter *m_outputBlack = nullptr;
    keira::DecimalParameter *m_outputWhite = nullptr;
};

} // namespace photon

#endif // PHOTON_CANVASLEVELSNODE_H
