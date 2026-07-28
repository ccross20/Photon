#ifndef PHOTON_CANVASGRADIENTNODE_H
#define PHOTON_CANVASGRADIENTNODE_H

#include "basecanvasnode.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

class GradientParameter;
class Point2DParameter;

// Canvas source node: fills the canvas with a multi-stop gradient (linear,
// radial, or angular). No input. Up to 16 stops are passed to the shader as
// uniforms. See [[canvas-gpu-pipeline]].
class PHOTONCORE_EXPORT CanvasGradientNode : public BaseCanvasNode
{
public:
    const static QByteArray Type;
    const static QByteArray GradientId;
    const static QByteArray Angle;
    const static QByteArray Center;
    const static QByteArray Radius;
    const static QByteArray Output;

    static const int MaxStops = 16;

    CanvasGradientNode();

    void createParameters() override;
    static keira::NodeInformation info();

protected:
    QByteArray fragmentShaderName() const override { return "gradient"; }
    quint32 uniformSize() const override { return 352; }   // header + shape + 4 pos vec4 + 16 colour vec4
    void writeUniforms(QByteArray &out, const QSize &size) const override;

private:
    keira::OptionParameter *m_type = nullptr;
    GradientParameter *m_gradient = nullptr;
    keira::DecimalParameter *m_angle = nullptr;
    Point2DParameter *m_center = nullptr;
    keira::DecimalParameter *m_radius = nullptr;
};

} // namespace photon

#endif // PHOTON_CANVASGRADIENTNODE_H
