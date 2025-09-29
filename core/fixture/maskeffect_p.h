#ifndef MASKEFFECT_P_H
#define MASKEFFECT_P_H

#include "maskeffect.h"

namespace photon {

class MaskEffect::Impl
{
public:
    QString name;
    QByteArray id;
    QByteArray uniqueId;
    MaskEffect *previousEffect = nullptr;
    QObject *parent = nullptr;
    QVector<Fixture*> fixtures;
};

}



#endif // MASKEFFECT_P_H
