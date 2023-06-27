#ifndef CLIPEFFECT_P_H
#define CLIPEFFECT_P_H

#include "clipeffect.h"

namespace photon {

class ClipEffect::Impl
{
public:
    ClipEffectEvaluationContext processFixture(Fixture *, ClipEffectEvaluationContext &, double);
    QVector<Channel*> channels;
    QString name;
    QByteArray id;
    QByteArray uniqueId;
    Clip *clip = nullptr;
};

}

#endif // CLIPEFFECT_P_H
