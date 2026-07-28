#ifndef PHOTON_CANVASGRADIENTMAPNODE_H
#define PHOTON_CANVASGRADIENTMAPNODE_H

#include "basecanvasnode.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/booleanparameter.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

class GradientParameter;

// Canvas effect node: remaps a Source texture through a gradient (a "gradient map").
// A chosen channel (luminance by default) drives the gradient lookup, so darks pick
// up the gradient's low colours and lights its high colours. Up to 16 stops are
// passed to the shader as uniforms. See [[canvas-gpu-pipeline]].
class PHOTONCORE_EXPORT CanvasGradientMapNode : public BaseCanvasNode
{
public:
    const static QByteArray Source;
    const static QByteArray GradientId;
    const static QByteArray Channel;
    const static QByteArray Invert;
    const static QByteArray Mix;
    const static QByteArray Output;

    static const int MaxStops = 16;

    CanvasGradientMapNode();

    void createParameters() override;
    static keira::NodeInformation info();

protected:
    QByteArray fragmentShaderName() const override { return "gradientmap"; }
    quint32 uniformSize() const override { return 336; }   // params + 4 pos vec4 + 16 colour vec4
    void writeUniforms(QByteArray &out, const QSize &size) const override;
    QVector<RhiTextureData> inputs() const override;

private:
    RhiTextureParameter *m_source = nullptr;
    GradientParameter *m_gradient = nullptr;
    keira::OptionParameter *m_channel = nullptr;
    keira::BooleanParameter *m_invert = nullptr;
    keira::DecimalParameter *m_mix = nullptr;
};

} // namespace photon

#endif // PHOTON_CANVASGRADIENTMAPNODE_H
