#include "palettetogradientnode.h"
#include "graph/parameter/colorpaletteparameter.h"
#include "graph/parameter/gradientparameter.h"
#include "util/gradient.h"

namespace photon {

const QByteArray PaletteToGradientNode::Palette = "palette";
const QByteArray PaletteToGradientNode::Output = "output";

keira::NodeInformation PaletteToGradientNode::info()
{
    keira::NodeInformation toReturn([](){return new PaletteToGradientNode;});
    toReturn.name = "Palette to Gradient";
    toReturn.nodeId = "photon.canvas.palette-to-gradient";
    toReturn.categories = {"Gradient"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

PaletteToGradientNode::PaletteToGradientNode() : keira::Node("photon.canvas.palette-to-gradient")
{
    setName("Palette to Gradient");
}

void PaletteToGradientNode::createParameters()
{
    ColorPalette defaultPalette{ QColor(255, 0, 0), QColor(0, 255, 0), QColor(0, 0, 255) };
    m_palette = new ColorPaletteParameter(Palette, "Palette", defaultPalette);
    addParameter(m_palette);

    m_output = new GradientParameter(Output, "Gradient", Gradient{}, keira::AllowMultipleOutput);
    addParameter(m_output);
}

void PaletteToGradientNode::evaluate(keira::EvaluationContext *) const
{
    const ColorPalette palette = m_palette->value().value<ColorPalette>();

    QVector<GradientStop> stops;
    stops.reserve(palette.size());

    if(palette.isEmpty())
    {
        m_output->setValue(QVariant::fromValue(Gradient{}));
        return;
    }

    if(palette.size() == 1)
    {
        // A single colour: a flat gradient (same colour at both ends).
        stops.append({0.0, palette.first()});
        stops.append({1.0, palette.first()});
    }
    else
    {
        const double step = 1.0 / double(palette.size() - 1);
        for(int i = 0; i < palette.size(); ++i)
            stops.append({i * step, palette.at(i)});
    }

    m_output->setValue(QVariant::fromValue(Gradient(stops)));
}

} // namespace photon
