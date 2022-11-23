#ifndef CLIP_P_H
#define CLIP_P_H

#include "clip.h"

namespace photon
{
    class Clip::Impl
    {
    public:
        Impl(Clip *);
        void setLayer(Layer *);
        void setSequence(Sequence *);
        void markChanged();
        void rebuildFalloffCache();
        double falloff(Fixture *);

        Layer *layer = nullptr;
        Sequence *sequence = nullptr;
        FixtureMask *mask = nullptr;
        QHash<Fixture*, double> cachedFalloffs;
        QVector<FalloffEffect*> falloffEffects;
        QVector<Channel*> channels;
        Routine *routine;
        double startTime = 0;
        double duration = 0;
        Clip *facade;
    };
}




#endif // CLIP_P_H
