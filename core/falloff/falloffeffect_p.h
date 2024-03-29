#ifndef FALLOFFEFFECT_P_H
#define FALLOFFEFFECT_P_H

#include "falloffeffect.h"

namespace photon {

class FalloffEffect::Impl
{
public:
    QString name;
    QByteArray id;
    QByteArray uniqueId;
    FixtureClip *clip = nullptr;
    FalloffEffect *previousEffect = nullptr;
};

}

#endif // FALLOFFEFFECT_P_H
