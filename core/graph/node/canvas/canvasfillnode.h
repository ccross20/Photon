#ifndef PHOTON_CANVASFILLNODE_H
#define PHOTON_CANVASFILLNODE_H

#include "basecanvasnode.h"
#include "model/parameter/optionparameter.h"

namespace photon {

class ColorParameter;

// Canvas source node: fills the canvas with a solid colour or a two-colour
// vertical/horizontal gradient. No input.
class PHOTONCORE_EXPORT CanvasFillNode : public BaseCanvasNode
{
public:
    const static QByteArray Color1;
    const static QByteArray Color2;
    const static QByteArray Mode;
    const static QByteArray Output;

    CanvasFillNode();

    void createParameters() override;
    static keira::NodeInformation info();

protected:
    QByteArray fragmentShaderName() const override { return "fill"; }
    quint32 uniformSize() const override { return 48; }   // 3 * vec4
    void writeUniforms(QByteArray &out, const QSize &size) const override;

private:
    ColorParameter *m_color1 = nullptr;
    ColorParameter *m_color2 = nullptr;
    keira::OptionParameter *m_mode = nullptr;
};

} // namespace photon

#endif // PHOTON_CANVASFILLNODE_H
