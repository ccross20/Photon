#include <QVBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include "randomfalloff.h"
#include "fixture/fixture.h"

namespace photon {

FalloffEffectInformation RandomFalloff::info()
{
    FalloffEffectInformation toReturn([](){return new RandomFalloff;});
    toReturn.name = "Random";
    toReturn.effectId = "photon.falloff.random";
    toReturn.categories.append("Modifier");

    return toReturn;
}


RandomFalloffEditor::RandomFalloffEditor(RandomFalloff *t_effect):QWidget(),m_effect(t_effect)
{

    QDoubleSpinBox *probSpin = new QDoubleSpinBox;
    probSpin->setValue(m_effect->probablity());
    connect(probSpin, &QDoubleSpinBox::valueChanged, this, &RandomFalloffEditor::probabilityChanged);

    QSpinBox *seedSpin = new QSpinBox;
    seedSpin->setMinimum(0);
    seedSpin->setValue(m_effect->seed());
    connect(seedSpin, &QSpinBox::valueChanged, this, &RandomFalloffEditor::seedChanged);


    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(new QLabel("Seed:"));
    vLayout->addWidget(seedSpin);
    vLayout->addStretch();
    setLayout(vLayout);
}

void RandomFalloffEditor::probabilityChanged(double t_value)
{
    m_effect->setProbablity(t_value);
}

void RandomFalloffEditor::seedChanged(int t_value)
{
    m_effect->setSeed(t_value);
}

RandomFalloff::RandomFalloff()
{

}

double RandomFalloff::falloff(Fixture *t_fixture) const
{
    if(previousEffect())
    {
        QRandomGenerator generator(static_cast<uint>(m_seed + (t_fixture->universe() * 512 + t_fixture->dmxOffset())));
        return previousEffect()->falloff(t_fixture) * generator.generateDouble();
    }
    return 0.0;
}

int RandomFalloff::seed() const
{
    return m_seed;
}

void RandomFalloff::setSeed(int t_value)
{
    if(m_seed == t_value)
        return;
    m_seed = t_value;
    updated();
}

double RandomFalloff::probablity() const
{
    return m_probablity;
}

void RandomFalloff::setProbablity(double t_value)
{
    if(m_probablity == t_value)
        return;
    m_probablity = t_value;
    updated();
}

QWidget *RandomFalloff::createEditor()
{
    return new RandomFalloffEditor(this);
}

void RandomFalloff::readFromJson(const QJsonObject &t_json)
{
    FalloffEffect::readFromJson(t_json);
    if(t_json.contains("seed"))
        m_seed = t_json.value("seed").toInt();
    if(t_json.contains("probability"))
        m_probablity = t_json.value("probability").toDouble();
}

void RandomFalloff::writeToJson(QJsonObject &t_json) const
{
    FalloffEffect::writeToJson(t_json);
    t_json.insert("seed", m_seed);
    t_json.insert("probability", m_probablity);
}

} // namespace photon
