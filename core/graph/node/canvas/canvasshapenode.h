#ifndef PHOTON_CANVASSHAPENODE_H
#define PHOTON_CANVASSHAPENODE_H

#include "basecanvasnode.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/decimalparameter.h"
#include "model/parameter/integerparameter.h"

namespace photon {

class Point2DParameter;
class ColorParameter;

// Canvas source node: draws a primitive shape via a signed-distance field (crisp,
// analytically anti-aliased, no CPU rasterisation). One node, with a Shape dropdown
// and generic transform/appearance parameters. Outputs the filled shape with a
// transparent exterior.
class PHOTONCORE_EXPORT CanvasShapeNode : public BaseCanvasNode
{
public:
    const static QByteArray Shape;
    const static QByteArray Position;
    const static QByteArray Size;
    const static QByteArray Rotation;
    const static QByteArray Roundness;
    const static QByteArray Sides;
    const static QByteArray Softness;
    const static QByteArray Fill;
    const static QByteArray StrokeColor;
    const static QByteArray StrokeWidth;
    const static QByteArray Output;

    CanvasShapeNode();

    void createParameters() override;
    static keira::NodeInformation info();

protected:
    QByteArray fragmentShaderName() const override { return "shape"; }
    quint32 uniformSize() const override { return 80; }   // 5 * vec4
    void writeUniforms(QByteArray &out, const QSize &size) const override;

private:
    keira::OptionParameter *m_shape = nullptr;
    Point2DParameter *m_position = nullptr;
    Point2DParameter *m_size = nullptr;
    keira::DecimalParameter *m_rotation = nullptr;
    keira::DecimalParameter *m_roundness = nullptr;
    keira::IntegerParameter *m_sides = nullptr;
    keira::DecimalParameter *m_softness = nullptr;
    ColorParameter *m_fill = nullptr;
    ColorParameter *m_strokeColor = nullptr;
    keira::DecimalParameter *m_strokeWidth = nullptr;
};

} // namespace photon

#endif // PHOTON_CANVASSHAPENODE_H
