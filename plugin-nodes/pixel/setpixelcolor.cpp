#include "setpixelcolor.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixture.h"
#include "fixture/capability/colorcapability.h"

namespace photon {


keira::NodeInformation SetPixelColor::info()
{
    keira::NodeInformation toReturn([](){return new SetPixelColor;});
    toReturn.name = "Set Pixel Color";
    toReturn.nodeId = "photon.plugin.node.set-pixel-color";
    toReturn.categories = {"Pixel"};
    toReturn.graphs = QByteArrayList{"pixel"};

    return toReturn;
}

SetPixelColor::SetPixelColor() : keira::Node("photon.plugin.node.set-pixel-color") {}


void SetPixelColor::parameterWasModified(keira::Parameter *t_param)
{
    if(t_param == m_priorityParam)
    {
        setPriority(m_priorityParam->value().toInt());
        markDirty(keira::Dirty_Priority);
    }
}

void SetPixelColor::createParameters()
{
    m_colorParam = new ColorParameter("colorInput","Color", Qt::white);
    addParameter(m_colorParam);

    m_pixelParam = new keira::IntegerParameter("pixelIndex","Pixel Index", 0);
    m_pixelParam->setMinimum(0);
    addParameter(m_pixelParam);

    m_blendParam = new keira::DecimalParameter("blendInput","Blend", 1.0);
    m_blendParam->setMinimum(0.0);
    m_blendParam->setMaximum(1.0);
    addParameter(m_blendParam);

    m_priorityParam = new keira::IntegerParameter("priority","Priority",0);
    addParameter(m_priorityParam);
}

void SetPixelColor::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->fixture)
    {

        auto colorCap = context->fixture->colorAtIndex(m_pixelParam->value().toInt());

        if(colorCap)
            static_cast<ColorCapability*>(colorCap)->setColor(m_colorParam->value().value<QColor>(), context->dmxMatrix, context->strength * m_blendParam->value().toDouble(),context->timeMachine);

    }

}

} // namespace photon
