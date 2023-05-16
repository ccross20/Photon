#ifndef PHOTON_RANDOMMASKEFFECT_H
#define PHOTON_RANDOMMASKEFFECT_H

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QRandomGenerator>
#include "fixture/maskeffect.h"

namespace photon {

class RandomMaskEffect;

class RandomMaskEditor :public QWidget
{
public:
    RandomMaskEditor(RandomMaskEffect *);

private slots:
    void seedUpdated(int);
    void probUpdated(double);

private:
    QSpinBox *m_seedSpin;
    QDoubleSpinBox *m_probSpin;
    RandomMaskEffect *m_effect;
};

class RandomMaskEffect : public MaskEffect
{
public:
    RandomMaskEffect();

    QVector<Fixture*> process(const QVector<Fixture*> fixtures) const override;
    QWidget *createEditor() override;
    void setSeed(int);
    int seed() const;
    void setProbability(double);
    double probability() const;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;


    static MaskEffectInformation info();

private:
    int m_seed = 0;
    double m_probability = .5;
};

} // namespace photon

#endif // PHOTON_RANDOMMASKEFFECT_H
