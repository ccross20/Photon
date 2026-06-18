
#include <QEasingCurve>
#include <QRandomGenerator>
#include "sparklenode.h"
#include "util/utils.h"
#include "routine/routineevaluationcontext.h"
#include "fixture/fixture.h"
#include "fixture/capability/colorcapability.h"

namespace photon {

class SparkleNode::Impl {
public:
    QEasingCurve m_easingIn;
    QEasingCurve m_easingOut;
    QRandomGenerator m_random;

};

keira::NodeInformation SparkleNode::info()
{
    keira::NodeInformation toReturn([](){return new SparkleNode;});
    toReturn.name = "Sparkle";
    toReturn.nodeId = "photon.color.sparkle";
    toReturn.categories = {"Color"};

    return toReturn;
}

SparkleNode::SparkleNode(): keira::Node("photon.color.sparkle"),m_impl(new Impl()) {}

SparkleNode::~SparkleNode()
{
    delete m_impl;
}

void SparkleNode::createParameters()
{
    m_colorParam = new ColorParameter("colorInput","Color", Qt::white);
    addParameter(m_colorParam);

    m_easeInTypeParam = new keira::OptionParameter("easeInType","Ease In Type",easeStrings(),0);
    addParameter(m_easeInTypeParam);
    m_easeInDurationParam = new keira::DecimalParameter("inDuration", "Ease In Duration", .2);
    m_easeInDurationParam->setMinimum(0.0);
    addParameter(m_easeInDurationParam);

    m_middleDurationParam = new keira::DecimalParameter("middleDuration", "Middle Duration", .2);
    m_middleDurationParam->setMinimum(0.0);
    addParameter(m_middleDurationParam);

    m_easeOutTypeParam = new keira::OptionParameter("easeOutType","Ease Out Type",easeStrings(),0);
    addParameter(m_easeOutTypeParam);
    m_easeOutDurationParam = new keira::DecimalParameter("outDuration", "Ease Out Duration", .2);
    m_easeOutDurationParam->setMinimum(0.0);
    addParameter(m_easeOutDurationParam);

    m_seedParam = new keira::IntegerParameter("seed", "Seed", 1);
    addParameter(m_seedParam);

    m_spreadParam = new keira::DecimalParameter("spread", "Spread", 1);
    addParameter(m_spreadParam);

    m_gapParam = new keira::DecimalParameter("gap", "Gap", 1);
    addParameter(m_gapParam);
}

void SparkleNode::evaluate(keira::EvaluationContext *t_context) const
{
    RoutineEvaluationContext *context = static_cast<RoutineEvaluationContext*>(t_context);
    if(context->fixture)
    {
        m_impl->m_easingIn.setType(static_cast<QEasingCurve::Type>(m_easeInTypeParam->value().toInt()));
        m_impl->m_easingOut.setType(static_cast<QEasingCurve::Type>(m_easeOutTypeParam->value().toInt()));

        double inDuration = m_easeInDurationParam->value().toDouble();
        double outDuration = m_easeOutDurationParam->value().toDouble();

        int seed = m_seedParam->value().toInt();
        double middleDuration = m_middleDurationParam->value().toDouble();
        double spread = m_spreadParam->value().toDouble();
        double gap = m_gapParam->value().toDouble();
        QColor color = m_colorParam->value().value<QColor>();

        float total = inDuration + middleDuration + outDuration + gap;


        int loopCount = std::floor((context->relativeTime + spread)/ (total + spread));


        auto colors = context->fixture->findCapability(Capability_Color);

        int index = 0;
        for(auto curColor : colors)
        {
            auto c = static_cast<ColorCapability*>(curColor);


            m_impl->m_random.seed((seed * loopCount+index));

            float t = fmod(context->relativeTime + (m_impl->m_random.generateDouble() * spread) , total+spread);
            double intensity = 0.0;

            if(t < inDuration)
                intensity = m_impl->m_easingIn.valueForProgress(t / inDuration);
            else if (t < inDuration + middleDuration)
                intensity = 1.0;
            else if (t < inDuration + middleDuration + outDuration)
                intensity = m_impl->m_easingOut.valueForProgress(1-((t - (inDuration + middleDuration)) / outDuration));

            //double intensity = (std::sin(M_PI * (t_context.relativeTime*speed + (m_offsets[index++] * spread)) )/2.0)+.5;
            index++;

            c->setColor(color,context->dmxMatrix, intensity * context->strength);
        }
    }
}

} // namespace photon
