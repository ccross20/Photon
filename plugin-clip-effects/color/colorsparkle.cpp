
#include "colorsparkle.h"
#include "channel/parameter/numberchannelparameter.h"
#include "channel/parameter/colorchannelparameter.h"
#include "channel/parameter/optionchannelparameter.h"
#include "fixture/fixture.h"
#include "fixture/capability/colorcapability.h"

namespace photon {

ColorSparkle::ColorSparkle() {

}

void ColorSparkle::init()
{
    BaseEffect::init();
    addChannelParameter(new NumberChannelParameter("seed"));
    addChannelParameter(new NumberChannelParameter("spread"));
    addChannelParameter(new NumberChannelParameter("gap"));
    addChannelParameter(new NumberChannelParameter("inDuration",.5,0));
    addChannelParameter(new NumberChannelParameter("middleDuration",.5,0));
    addChannelParameter(new NumberChannelParameter("outDuration",.5,0));
    addChannelParameter(new ColorChannelParameter("color"));

    QStringList easeStrings;
    easeStrings << "Linear";
    easeStrings << "In Quad";
    easeStrings << "Out Quad";
    easeStrings << "In Out Quad";
    easeStrings << "Out In Quad";

    easeStrings << "In Cubic";
    easeStrings << "Out Cubic";
    easeStrings << "In Out Cubic";
    easeStrings << "Out In Cubic";

    easeStrings << "In Quart";
    easeStrings << "Out Quart";
    easeStrings << "In Out Quart";
    easeStrings << "Out In Quart";

    easeStrings << "In Quint";
    easeStrings << "Out Quint";
    easeStrings << "In Out Quint";
    easeStrings << "Out In Quint";

    easeStrings << "In Sine";
    easeStrings << "Out Sine";
    easeStrings << "In Out Sine";
    easeStrings << "Out In Sine";

    easeStrings << "In Expo";
    easeStrings << "Out Expo";
    easeStrings << "In Out Expo";
    easeStrings << "Out In Expo";

    easeStrings << "In Circ";
    easeStrings << "Out Circ";
    easeStrings << "In Out Circ";
    easeStrings << "Out In Circ";

    easeStrings << "In Elastic";
    easeStrings << "Out Elastic";
    easeStrings << "In Out Elastic";
    easeStrings << "Out In Elastic";

    easeStrings << "In Back";
    easeStrings << "Out Back";
    easeStrings << "In Out Back";
    easeStrings << "Out In Back";

    easeStrings << "In Bounce";
    easeStrings << "Out Bounce";
    easeStrings << "In Out Bounce";
    easeStrings << "Out In Bounce";


    addChannelParameter(new OptionChannelParameter("easeInMode", easeStrings, 3));
    addChannelParameter(new OptionChannelParameter("easeOutMode", easeStrings, 3));

    QRandomGenerator rd;
    for(int i = 0; i < 10; ++i)
        m_offsets.append(rd.generateDouble());

}

void ColorSparkle::evaluate(FixtureEffectEvaluationContext &t_context)
{
    int seed = t_context.channelValues["seed"].toInt();
    float spread = t_context.channelValues["spread"].toFloat();
    float inDuration = t_context.channelValues["inDuration"].toFloat();
    float middleDuration = t_context.channelValues["middleDuration"].toFloat();
    float outDuration = t_context.channelValues["outDuration"].toFloat();
    float gap = t_context.channelValues["gap"].toFloat();
    QColor color = t_context.channelValues["color"].value<QColor>();
    int easeInMode = t_context.channelValues["easeInMode"].toInt();
    int easeOutMode = t_context.channelValues["easeOutMode"].toInt();

    auto fixture = t_context.fixture;
    auto colors = fixture->findCapability(Capability_Color);

    //qDebug() << color;
    //color = QColor::fromRgb(124,254,23);

    m_easingOut.setType(static_cast<QEasingCurve::Type>(easeOutMode));
    m_easingIn.setType(static_cast<QEasingCurve::Type>(easeInMode));

    float total = inDuration + middleDuration + outDuration + gap;


    int loopCount = std::floor((t_context.relativeTime + spread)/ (total + spread));

    int index = 0;
    for(auto curColor : colors)
    {
        auto c = static_cast<ColorCapability*>(curColor);


        m_random.seed((seed * loopCount+index));

        float t = fmod(t_context.relativeTime + (m_random.generateDouble() * spread) , total+spread);
        double intensity = 0.0;

        if(t < inDuration)
            intensity = m_easingIn.valueForProgress(t / inDuration);
        else if (t < inDuration + middleDuration)
            intensity = 1.0;
        else if (t < inDuration + middleDuration + outDuration)
            intensity = m_easingOut.valueForProgress(1-((t - (inDuration + middleDuration)) / outDuration));

        //double intensity = (std::sin(M_PI * (t_context.relativeTime*speed + (m_offsets[index++] * spread)) )/2.0)+.5;
        index++;

        c->setColor(color,t_context.dmxMatrix, intensity * t_context.strength);
    }
}

ClipEffectInformation ColorSparkle::info()
{
    ClipEffectInformation toReturn([](){return new ColorSparkle;});
    toReturn.name = "Color Sparkle";
    toReturn.id = "photon.clip.color-sparkle";
    toReturn.categories.append("Color");

    return toReturn;
}

} // namespace photon
