#ifndef PHOTON_CANVASNOISENODE_H
#define PHOTON_CANVASNOISENODE_H

#include "basecanvasnode.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"

namespace photon {

class Point2DParameter;

// Canvas source node: procedural greyscale noise. Modes (Value / Fractal / Cellular),
// an offset to pan it, scale (frequency) and complexity (fractal octaves), plus a
// Time input to drive it through a third dimension for animation.
class PHOTONCORE_EXPORT CanvasNoiseNode : public BaseCanvasNode
{
public:
    const static QByteArray Mode;
    const static QByteArray Offset;
    const static QByteArray Scale;
    const static QByteArray Complexity;
    const static QByteArray Warp;
    const static QByteArray Seed;
    const static QByteArray Time;
    const static QByteArray Output;

    CanvasNoiseNode();

    void createParameters() override;
    static keira::NodeInformation info();

protected:
    QByteArray fragmentShaderName() const override { return "noise"; }
    quint32 uniformSize() const override { return 48; }   // 3 * vec4
    void writeUniforms(QByteArray &out, const QSize &size) const override;

private:
    keira::OptionParameter *m_mode = nullptr;
    Point2DParameter *m_offset = nullptr;
    keira::DecimalParameter *m_scale = nullptr;
    keira::IntegerParameter *m_complexity = nullptr;
    keira::DecimalParameter *m_warp = nullptr;
    keira::IntegerParameter *m_seed = nullptr;
    keira::DecimalParameter *m_time = nullptr;
};

} // namespace photon

#endif // PHOTON_CANVASNOISENODE_H
