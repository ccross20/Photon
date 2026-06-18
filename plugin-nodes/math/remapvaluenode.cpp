#include <QEasingCurve>
#include "remapvaluenode.h"
#include "util/utils.h"

namespace photon {

class RemapValueNode::Impl
{
public:
    QEasingCurve m_easing;
};

keira::NodeInformation RemapValueNode::info()
{
    keira::NodeInformation toReturn([](){return new RemapValueNode;});
    toReturn.name = "Remap";
    toReturn.nodeId = "photon.math.remap";
    toReturn.categories = {"Math"};

    return toReturn;
}

RemapValueNode::RemapValueNode(): keira::Node("photon.math.remap"),m_impl(new Impl()) {}

RemapValueNode::~RemapValueNode()
{
    delete m_impl;
}

void RemapValueNode::createParameters()
{
    m_inputParam = new keira::DecimalParameter("input", "Input", 0);
    addParameter(m_inputParam);

    m_minInParam = new keira::DecimalParameter("minInput", "Min Input", 0);
    addParameter(m_minInParam);

    m_maxInParam = new keira::DecimalParameter("maxInput", "Max Input", 0);
    addParameter(m_maxInParam);

    m_easeTypeParam = new keira::OptionParameter("easeInType","Ease Type",easeStrings(),0);
    addParameter(m_easeTypeParam);



    m_minOutParam = new keira::DecimalParameter("minOutput", "Min Output", 0);
    addParameter(m_minOutParam);

    m_maxOutParam = new keira::DecimalParameter("maxOutput", "Max Output", 0);
    addParameter(m_maxOutParam);


    m_boundsParam = new keira::OptionParameter("bounds","Bounds Mode",{"Limit","Loop","Ping Pong"},0);
    addParameter(m_boundsParam);


    m_outputParam = new keira::DecimalParameter("output", "Output", 0, keira::AllowMultipleOutput);
    addParameter(m_outputParam);


}

void RemapValueNode::evaluate(keira::EvaluationContext *t_context) const
{
    m_impl->m_easing.setType(static_cast<QEasingCurve::Type>(m_easeTypeParam->value().toInt()));

    double inputValue = m_inputParam->value().toDouble();
    double inMin = m_minInParam->value().toDouble();
    double inMax = m_maxInParam->value().toDouble();
    double outMin = m_minOutParam->value().toDouble();
    double outMax = m_maxOutParam->value().toDouble();
    int boundsType = m_boundsParam->value().toInt();

    double inDelta = inMax - inMin;
    double outDelta = outMax - outMin;
    double mappedIn = mappedIn = (inputValue-inMin)/inDelta;

    switch (boundsType) {
        default:
        case 0:
        {
            if(inputValue > inMax)
                inputValue = inMax;
            if(inputValue < inMin)
                inputValue = inMin;
            mappedIn = (inputValue-inMin)/inDelta;
        }
            break;
        case 1:
            mappedIn = mappedIn - (floor(mappedIn));
        break;
        case 2:
        {
            double wrapped = std::fmod(mappedIn, 2.0);

            // std::abs(wrapped - 1.0) flips the second half of the cycle
            mappedIn = 1.0 - std::abs(wrapped - 1.0);
        }
            break;
    }



    double output = (m_impl->m_easing.valueForProgress(mappedIn) * outDelta) + outMin;

    m_outputParam->setValue(output);
}

} // namespace photon
