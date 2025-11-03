#ifndef BASEEFFECTPARENT_H
#define BASEEFFECTPARENT_H
#include <QVector>
#include "photon-global.h"

namespace photon {

class BaseEffect;

class BaseEffectParent
{
public:
    virtual ~BaseEffectParent(){}

    virtual void markChanged() = 0;
    virtual double startTime() const = 0;
    virtual double duration() const = 0;
    virtual void effectUpdated(BaseEffect *) = 0;
    virtual double strengthAtTime(double) const = 0;

    virtual void addClipEffect(BaseEffect *) = 0;
    virtual void removeClipEffect(BaseEffect *) = 0;
    virtual BaseEffect *clipEffectAtIndex(int index) const = 0;
    virtual int clipEffectCount() const = 0;
    virtual const QVector<BaseEffect*> &clipEffects() const = 0;
};

}


#endif // BASEEFFECTPARENT_H
