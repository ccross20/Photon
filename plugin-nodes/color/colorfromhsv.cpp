#include "colorfromhsv.h"

namespace photon {


keira::NodeInformation ColorFromHSV::info()
{
    keira::NodeInformation toReturn([](){return new ColorFromHSV;});
    toReturn.name = "Color from HSV";
    toReturn.nodeId = "photon.color.color_from_hsv";
    toReturn.categories = {"Color"};

    return toReturn;
}

ColorFromHSV::ColorFromHSV() : keira::Node("photon.color.color_from_hsv")
{

}

void ColorFromHSV::createParameters()
{
    hueParam = new keira::DecimalParameter("hue","Hue", 1.0);
    addParameter(hueParam);

    satParam = new keira::DecimalParameter("sat","Saturation", 1.0);
    addParameter(satParam);

    valueParam = new keira::DecimalParameter("value","Value", 1.0);
    addParameter(valueParam);

    outputParam = new ColorParameter("output","Output", 0.0, keira::AllowMultipleOutput);
    addParameter(outputParam);
}

void ColorFromHSV::evaluate(keira::EvaluationContext *t_context) const
{
    double hue = hueParam->value().toDouble();
    double sat = satParam->value().toDouble();
    double val = valueParam->value().toDouble();

    if(hue > 1.0)
        hue -= std::floor(hue);
    if(sat > 1.0)
        sat -= std::floor(sat);
    if(val > 1.0)
        val -= std::floor(val);


    outputParam->setValue(QColor::fromHsvF(std::max(0.0,hue),
                                           std::max(0.0,sat),
                                           std::max(0.0,val)));
}

} // namespace photon
