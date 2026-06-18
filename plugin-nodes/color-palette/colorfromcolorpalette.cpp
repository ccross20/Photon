#include "colorfromcolorpalette.h"

namespace photon {


keira::NodeInformation ColorFromColorPalette::info()
{
    keira::NodeInformation toReturn([](){return new ColorFromColorPalette;});
    toReturn.name = "Color from Palette";
    toReturn.nodeId = "photon.palette.select-color";
    toReturn.categories = {"Color Palette"};

    return toReturn;
}

ColorFromColorPalette::ColorFromColorPalette() : keira::Node("photon.palette.select-color") {}



void ColorFromColorPalette::createParameters()
{


    m_indexParam = new keira::IntegerParameter("index","Index", 0);
    m_indexParam->setMinimum(0);
    addParameter(m_indexParam);

    m_paletteParam = new ColorPaletteParameter("palette", "Palette", ColorPalette{});
    addParameter(m_paletteParam);

    m_colorParam = new ColorParameter("color", "Color", Qt::black, keira::AllowMultipleOutput);
    addParameter(m_colorParam);
}

void ColorFromColorPalette::evaluate(keira::EvaluationContext *t_context) const
{
    auto index = m_indexParam->value().toLongLong();
    ColorPalette palette = m_paletteParam->value().value<ColorPalette>();

    if(palette.isEmpty())
        return;

    index = index % palette.length();

    m_colorParam->setValue(palette[fmax(0,fmin(index,palette.length()-1))]);
}

} // namespace photon
