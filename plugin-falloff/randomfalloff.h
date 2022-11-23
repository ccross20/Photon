#ifndef PHOTON_RANDOMFALLOFF_H
#define PHOTON_RANDOMFALLOFF_H
#include <QWidget>
#include <QRandomGenerator>
#include "falloff/falloffeffect.h"

namespace photon {

class RandomFalloff;

class RandomFalloffEditor : public QWidget
{
public:
    RandomFalloffEditor(RandomFalloff *);


private slots:
    void probabilityChanged(double);
    void seedChanged(int);

private:
    RandomFalloff *m_effect;
};

class RandomFalloff : public FalloffEffect
{
public:
    RandomFalloff();

    double falloff(Fixture *) const override;
    QWidget *createEditor() override;

    double probablity() const;
    void setProbablity(double);

    int seed() const;
    void setSeed(int);

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static FalloffEffectInformation info();

private:
    int m_seed = 0;
    double m_probablity;
};

} // namespace photon

#endif // PHOTON_RANDOMFALLOFF_H
