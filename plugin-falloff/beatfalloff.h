#ifndef PHOTON_BEATFALLOFF_H
#define PHOTON_BEATFALLOFF_H


#include "falloff/falloffeffect.h"

namespace photon {

class BeatFalloff : public FalloffEffect
{
public:
    BeatFalloff();

    double falloff(Fixture *) const override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;


    static FalloffEffectInformation info();

private:
    void rebuildCache();

};

} // namespace photon

#endif // PHOTON_BEATFALLOFF_H
