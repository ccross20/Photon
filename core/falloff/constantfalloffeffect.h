#ifndef PHOTON_CONSTANTFALLOFFEFFECT_H
#define PHOTON_CONSTANTFALLOFFEFFECT_H

#include <QWidget>
#include "falloffeffect.h"

namespace photon {

class ConstantFalloffEffect;

class ConstantFalloffEditor : public QWidget
{
public:
    ConstantFalloffEditor(ConstantFalloffEffect *);


private slots:
    void valueChanged(double);

private:
    ConstantFalloffEffect *m_effect;
};

class ConstantFalloffEffect : public FalloffEffect
{
public:
    ConstantFalloffEffect();

    double falloff(Fixture *) const override;
    QWidget *createEditor() override;

    double value() const;
    void setValue(double);

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static FalloffEffectInformation info();

private:
    double m_value = 0.0;
};

} // namespace photon

#endif // PHOTON_CONSTANTFALLOFFEFFECT_H
