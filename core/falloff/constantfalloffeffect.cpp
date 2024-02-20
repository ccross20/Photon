#include <QVBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include "constantfalloffeffect.h"
#include "sequence/fixtureclip.h"

namespace photon {


FalloffEffectInformation ConstantFalloffEffect::info()
{
    FalloffEffectInformation toReturn([](){return new ConstantFalloffEffect;});
    toReturn.name = "Constant Value";
    toReturn.effectId = "photon.falloff.constant";
    toReturn.categories.append("Generator");

    return toReturn;
}


ConstantFalloffEditor::ConstantFalloffEditor(ConstantFalloffEffect *t_effect):QWidget(),m_effect(t_effect)
{

    QDoubleSpinBox *valueSpin = new QDoubleSpinBox;
    valueSpin->setMinimum(0);
    valueSpin->setMaximum(60);
    valueSpin->setValue(m_effect->value());
    connect(valueSpin, &QDoubleSpinBox::valueChanged, this, &ConstantFalloffEditor::valueChanged);


    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addWidget(new QLabel("Value:"));
    vLayout->addWidget(valueSpin);
    vLayout->addStretch();
    setLayout(vLayout);
}

void ConstantFalloffEditor::valueChanged(double t_value)
{
    m_effect->setValue(t_value);
}

ConstantFalloffEffect::ConstantFalloffEffect()
{

}

double ConstantFalloffEffect::falloff(Fixture *t_fixture) const
{
    return clip()->defaultFalloff();
}

double ConstantFalloffEffect::value() const
{
    return clip()->defaultFalloff();
}

void ConstantFalloffEffect::setValue(double t_value)
{
    clip()->setDefaultFalloff(t_value);
    updated();
}

QWidget *ConstantFalloffEffect::createEditor()
{
    return new ConstantFalloffEditor(this);
}

void ConstantFalloffEffect::readFromJson(const QJsonObject &t_json)
{
    FalloffEffect::readFromJson(t_json);
}

void ConstantFalloffEffect::writeToJson(QJsonObject &t_json) const
{
    FalloffEffect::writeToJson(t_json);
}

} // namespace photon
