#include "colorpalettenode.h"
#include "graph/parameter/colorpaletteparameter.h"

namespace photon {

const QByteArray ColorPaletteNode::PaletteParam = "palette";


keira::NodeInformation ColorPaletteNode::info()
{
    keira::NodeInformation toReturn([](){return new ColorPaletteNode;});
    toReturn.name = "Color Palette";
    toReturn.nodeId = "photon.color-palette.create";
    toReturn.categories = {"Color Palette"};

    return toReturn;
}

ColorPaletteNode::ColorPaletteNode() : keira::Node("photon.color-palette.create") {}



void ColorPaletteNode::createParameters()
{

    addParameter(new ColorPaletteParameter(PaletteParam, "Palette", ColorPalette{}));

}


}

