#ifndef CLIPLAYER_P_H
#define CLIPLAYER_P_H

#include "cliplayer.h"

namespace photon
{

class ClipLayer::Impl
{
public:
    Impl(ClipLayer *);
    void addClip(Clip *);
    void removeClip(Clip *);
    void notifyClipWasModified(Clip *);
    QVector<Clip *> clips;
    ClipLayer *facade;
};

}

#endif // CLIPLAYER_P_H
