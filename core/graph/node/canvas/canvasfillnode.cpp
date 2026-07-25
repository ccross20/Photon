#include <QColor>
#include "canvasfillnode.h"
#include "graph/parameter/colorparameter.h"

namespace photon {

const QByteArray CanvasFillNode::Color1 = "color";
const QByteArray CanvasFillNode::Color2 = "color2";
const QByteArray CanvasFillNode::Mode = "mode";
const QByteArray CanvasFillNode::Output = "output";

keira::NodeInformation CanvasFillNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasFillNode;});
    toReturn.name = "Fill";
    toReturn.nodeId = "photon.canvas.fill";
    toReturn.categories = {"Canvas"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

CanvasFillNode::CanvasFillNode() : BaseCanvasNode("photon.canvas.fill")
{
    setName("Fill");
}

void CanvasFillNode::createParameters()
{
    m_color1 = new ColorParameter(Color1, "Color", QColor(255, 255, 255, 255));
    addParameter(m_color1);

    m_color2 = new ColorParameter(Color2, "Color 2", QColor(0, 0, 0, 255));
    addParameter(m_color2);

    m_mode = new keira::OptionParameter(Mode, "Mode", {"Solid", "Vertical", "Horizontal"}, 0);
    addParameter(m_mode);

    m_outputParam = new RhiTextureParameter(Output, "Canvas", RhiTextureData{}, keira::AllowMultipleOutput);
    addParameter(m_outputParam);
}

void CanvasFillNode::writeUniforms(QByteArray &out, const QSize &) const
{
    float *f = reinterpret_cast<float *>(out.data());
    const QColor c1 = m_color1->value().value<QColor>();
    const QColor c2 = m_color2->value().value<QColor>();

    f[0] = float(c1.redF());   f[1] = float(c1.greenF()); f[2] = float(c1.blueF());  f[3] = float(c1.alphaF());
    f[4] = float(c2.redF());   f[5] = float(c2.greenF()); f[6] = float(c2.blueF());  f[7] = float(c2.alphaF());
    f[8] = float(m_mode->value().toInt());
    f[9] = 0.0f; f[10] = 0.0f; f[11] = 0.0f;
}

} // namespace photon
