#ifndef PHOTON_CANVASWIPENODE_H
#define PHOTON_CANVASWIPENODE_H

#include "basecanvasnode.h"
#include "model/parameter/decimalparameter.h"

namespace photon {

// Canvas effect node: a linear wipe between two textures. Progress drives a line
// that reveals "To" over "From"; the line can be rotated (Angle) and softened. With
// zero softness the edge is anti-aliased (from the coordinate's screen-space
// gradient), so a rotated hard edge stays clean.
class PHOTONCORE_EXPORT CanvasWipeNode : public BaseCanvasNode
{
public:
    const static QByteArray From;
    const static QByteArray To;
    const static QByteArray Progress;
    const static QByteArray Angle;
    const static QByteArray Softness;
    const static QByteArray Output;

    CanvasWipeNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

protected:
    QByteArray fragmentShaderName() const override { return "wipe"; }
    quint32 uniformSize() const override { return 16; }   // 1 * vec4
    void writeUniforms(QByteArray &out, const QSize &size) const override;
    QVector<RhiTextureData> inputs() const override;

private:
    RhiTextureParameter *m_from = nullptr;
    RhiTextureParameter *m_to = nullptr;
    keira::DecimalParameter *m_progress = nullptr;
    keira::DecimalParameter *m_angle = nullptr;
    keira::DecimalParameter *m_softness = nullptr;
};

} // namespace photon

#endif // PHOTON_CANVASWIPENODE_H
