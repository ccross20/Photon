#include "randomnumbernode.h"

namespace photon {

class RandomNumberNode::Impl{
public:
    QRandomGenerator m_random;
    int m_lastSeed = 0;
    double m_lastValue = 0.0;
};

keira::NodeInformation RandomNumberNode::info()
{
    keira::NodeInformation toReturn([](){return new RandomNumberNode;});
    toReturn.name = "Random Number";
    toReturn.nodeId = "photon.math.random";
    toReturn.categories = {"Math"};

    return toReturn;
}

RandomNumberNode::RandomNumberNode(): keira::Node("photon.math.random"),m_impl(new Impl()) {}

RandomNumberNode::~RandomNumberNode()
{
    delete m_impl;
}

void RandomNumberNode::createParameters()
{
    m_modeParam = new keira::OptionParameter("mode","Mode",{"Integer","Decimal"},0);
    addParameter(m_modeParam);

    m_seedParam = new keira::IntegerParameter("seed","Seed", 1);
    m_seedParam->setMinimum(0);
    addParameter(m_seedParam);

    m_ensureDifferent = new keira::BooleanParameter("diff", "Ensure Different", true);
    addParameter(m_ensureDifferent);

    m_minParam = new keira::DecimalParameter("min", "Min", 0);
    addParameter(m_minParam);

    m_maxParam = new keira::DecimalParameter("max", "Max", 10000);
    addParameter(m_maxParam);


    m_outputParam = new keira::DecimalParameter("output", "Output", 0, keira::AllowMultipleOutput);
    addParameter(m_outputParam);


}

void RandomNumberNode::evaluate(keira::EvaluationContext *t_context) const
{
    int seed = m_seedParam->value().toInt();


    if(m_impl->m_lastSeed != seed)
    {
        m_impl->m_random.seed(seed);
        int mode = m_modeParam->value().toInt();
        double tempMin = m_minParam->value().toDouble();
        double tempMax = m_maxParam->value().toDouble();

        double min = std::min(tempMin,tempMax);
        double max = std::max(tempMin,tempMax);

        double current = m_outputParam->value().toDouble();
        double result = ((max - min) * m_impl->m_random.generateDouble()) + min;

        if(mode == 0)
        {
            result = std::round(result);
        }

        if(m_ensureDifferent->value().toBool())
        {
            if((mode == 0 && abs(max-min) > 2) || (mode == 1 && min != max))
            {
                int seedCounter = 10000000;
                while(result == current && seedCounter < 10000020)
                {
                    m_impl->m_random.seed(seed + seedCounter);
                    result = ((max - min) * m_impl->m_random.generateDouble()) + min;
                    seedCounter += 1;

                    if(mode == 0)
                    {
                        result = std::round(result);
                    }

                }
            }
        }

        m_impl->m_lastValue = result;


        m_outputParam->setValue(result);
    }



    m_impl->m_lastSeed = seed;

}

} // namespace photon
