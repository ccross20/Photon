#ifndef PHOTON_CANVASMASKNODE_H
#define PHOTON_CANVASMASKNODE_H

#include "basecanvasnode.h"
#include "model/parameter/optionparameter.h"
#include "model/parameter/booleanparameter.h"

namespace photon {

// Canvas effect node: masks a Source texture with a Mask texture. The mask's chosen
// channel (Alpha by default) multiplies the source's alpha — cutting it out. With
// only the source wired it passes through unchanged.
class PHOTONCORE_EXPORT CanvasMaskNode : public BaseCanvasNode
{
public:
    const static QByteArray Source;
    const static QByteArray Mask;
    const static QByteArray Channel;
    const static QByteArray Invert;
    const static QByteArray Output;

    CanvasMaskNode();

    void createParameters() override;
    void evaluate(keira::EvaluationContext *) const override;
    static keira::NodeInformation info();

protected:
    QByteArray fragmentShaderName() const override { return "mask"; }
    quint32 uniformSize() const override { return 16; }   // 1 * vec4
    void writeUniforms(QByteArray &out, const QSize &size) const override;
    QVector<RhiTextureData> inputs() const override;

private:
    RhiTextureParameter *m_source = nullptr;
    RhiTextureParameter *m_mask = nullptr;
    keira::OptionParameter *m_channel = nullptr;
    keira::BooleanParameter *m_invert = nullptr;
};

} // namespace photon

#endif // PHOTON_CANVASMASKNODE_H
