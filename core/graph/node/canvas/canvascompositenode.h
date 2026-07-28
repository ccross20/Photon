#ifndef PHOTON_CANVASCOMPOSITENODE_H
#define PHOTON_CANVASCOMPOSITENODE_H

#include "basecanvasnode.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

class Point2DParameter;

// Canvas effect node: composites a Top texture over a Base texture with a blend
// mode and alpha. The top can be positioned, scaled, rotated and pivoted (origin)
// in the canvas. With only one input wired it passes that input through.
class PHOTONCORE_EXPORT CanvasCompositeNode : public BaseCanvasNode
{
public:
    const static QByteArray Base;
    const static QByteArray Top;
    const static QByteArray BlendMode;
    const static QByteArray Alpha;
    const static QByteArray Position;
    const static QByteArray Scale;
    const static QByteArray Rotation;
    const static QByteArray Origin;
    const static QByteArray Output;

    CanvasCompositeNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

protected:
    QByteArray fragmentShaderName() const override { return "composite"; }
    quint32 uniformSize() const override { return 48; }   // 3 * vec4
    void writeUniforms(QByteArray &out, const QSize &size) const override;
    QVector<RhiTextureData> inputs() const override;

private:
    RhiTextureParameter *m_base = nullptr;
    RhiTextureParameter *m_top = nullptr;
    keira::OptionParameter *m_blendMode = nullptr;
    keira::DecimalParameter *m_alpha = nullptr;
    Point2DParameter *m_position = nullptr;
    Point2DParameter *m_scale = nullptr;
    keira::DecimalParameter *m_rotation = nullptr;
    Point2DParameter *m_origin = nullptr;
};

} // namespace photon

#endif // PHOTON_CANVASCOMPOSITENODE_H
