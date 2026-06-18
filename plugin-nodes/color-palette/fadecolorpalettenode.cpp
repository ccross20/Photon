#include "fadecolorpalettenode.h"
#include "util/utils.h"

namespace photon {


keira::NodeInformation FadeColorPaletteNode::info()
{
    keira::NodeInformation toReturn([](){return new FadeColorPaletteNode;});
    toReturn.name = "Fade Palette";
    toReturn.nodeId = "photon.palette.fade-colors";
    toReturn.categories = {"Fade Colors"};

    return toReturn;
}

FadeColorPaletteNode::FadeColorPaletteNode() : keira::Node("photon.palette.fade-colors") {}



void FadeColorPaletteNode::createParameters()
{
    m_modeParam = new keira::OptionParameter("mode","Mode",{"HSL","RGB"},0);
    addParameter(m_modeParam);

    m_blendParam = new keira::DecimalParameter("blend","Blend", 0.0);
    addParameter(m_blendParam);

    m_paletteParam = new ColorPaletteParameter("palette", "Palette", ColorPalette{});
    addParameter(m_paletteParam);

    m_colorParam = new ColorParameter("color", "Color", Qt::black, keira::AllowMultipleOutput);
    addParameter(m_colorParam);
}

void FadeColorPaletteNode::evaluate(keira::EvaluationContext *t_context) const
{
    double blend = m_blendParam->value().toDouble();
    int mode = m_modeParam->value().toInt();


    ColorPalette palette = m_paletteParam->value().value<ColorPalette>();

    if(palette.isEmpty())
        return;

    blend *= (palette.length()-1);

    int aIndex = int(floor(blend)) % palette.length();
    int bIndex = int(ceil(blend)) % palette.length();

    blend -= floor(blend);

    if(aIndex < 0)
        aIndex = 0;
    if(aIndex >= palette.length())
        aIndex = palette.length()-1;
    if(bIndex < 0)
        bIndex = 0;
    if(bIndex >= palette.length())
        bIndex = palette.length()-1;

    if(mode == 0)
    {
        m_colorParam->setValue(blendColorsHSL(palette[aIndex],palette[bIndex],blend));
    }
    else
    {
        m_colorParam->setValue(blendColors(palette[aIndex],palette[bIndex],blend));
    }


}

} // namespace photon

