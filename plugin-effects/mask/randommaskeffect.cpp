#include <QFormLayout>
#include "randommaskeffect.h"

namespace photon {

RandomMaskEditor::RandomMaskEditor(RandomMaskEffect *t_effect):m_effect(t_effect)
{
    QFormLayout *form = new QFormLayout;

    m_seedSpin = new QSpinBox;
    m_seedSpin->setMinimum(0);
    m_seedSpin->setMaximum(9999);
    m_seedSpin->setValue(t_effect->seed());
    form->addRow("Seed", m_seedSpin);

    m_probSpin = new QDoubleSpinBox;
    m_probSpin->setMinimum(0);
    m_probSpin->setMaximum(1.0);
    m_probSpin->setValue(t_effect->probability());
    form->addRow("Probability", m_probSpin);


    setLayout(form);

    connect(m_seedSpin, &QSpinBox::valueChanged, this, &RandomMaskEditor::seedUpdated);
    connect(m_probSpin, &QDoubleSpinBox::valueChanged, this, &RandomMaskEditor::probUpdated);
}

void RandomMaskEditor::seedUpdated(int t_value)
{
    m_effect->setSeed(t_value);
}

void RandomMaskEditor::probUpdated(double t_value)
{
    m_effect->setProbability(t_value);
}

RandomMaskEffect::RandomMaskEffect() : MaskEffect("photon.core.random-mask")
{

}

QVector<Fixture*> RandomMaskEffect::process(const QVector<Fixture*> fixtures) const
{
    QVector<Fixture*> results;

    QRandomGenerator generator(static_cast<uint>(m_seed));

    for(auto fix : fixtures)
    {
        if(generator.generateDouble() < m_probability)
            results.append(fix);
    }

    return results;
}

QWidget *RandomMaskEffect::createEditor()
{
    return new RandomMaskEditor(this);
}

void RandomMaskEffect::setSeed(int t_value)
{
    m_seed = t_value;
    updated();
}

int RandomMaskEffect::seed() const
{
    return m_seed;
}

void RandomMaskEffect::setProbability(double t_value)
{
    m_probability = t_value;
    updated();
}

double RandomMaskEffect::probability() const
{
    return m_probability;
}


void RandomMaskEffect::readFromJson(const QJsonObject &t_json)
{
    MaskEffect::readFromJson(t_json);
}

void RandomMaskEffect::writeToJson(QJsonObject &t_json) const
{
    MaskEffect::writeToJson(t_json);
}



MaskEffectInformation RandomMaskEffect::info()
{
    MaskEffectInformation toReturn([](){return new RandomMaskEffect;});
    toReturn.name = "Random Mask";
    toReturn.effectId = "photon.core.random-mask";

    return toReturn;
}


} // namespace photon
