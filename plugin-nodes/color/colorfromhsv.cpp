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
    // 0-1 is the normal range for all three, so that's the slider's soft
    // range - but left hard-unbounded, since evaluate() below already wraps
    // hue past 1 and floors sat/value at 0 rather than rejecting them, e.g. a
    // continuously-rotating hue driven by an LFO node is a legitimate use.
    hueParam = new keira::DecimalParameter("hue","Hue", 1.0);
    hueParam->setSoftRange(0.0, 1.0);
    addParameter(hueParam);

    satParam = new keira::DecimalParameter("sat","Saturation", 1.0);
    satParam->setSoftRange(0.0, 1.0);
    addParameter(satParam);

    valueParam = new keira::DecimalParameter("value","Value", 1.0);
    valueParam->setSoftRange(0.0, 1.0);
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
