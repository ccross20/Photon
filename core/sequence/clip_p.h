#ifndef CLIP_P_H
#define CLIP_P_H

#include "clip.h"

namespace photon
{
    class Clip::Impl
    {
    public:
        Impl(Clip *);
        void setLayer(ClipLayer *);
        void setSequence(Sequence *);
        void markChanged();
        double falloff(Fixture *);

        ClipLayer *layer = nullptr;
        Sequence *sequence = nullptr;
        QVector<MaskEffect*> maskEffects;
        QVector<FalloffEffect*> falloffEffects;
        QVector<Channel*> channels;
        QByteArray type;
        QByteArray uniqueId;
        QEasingCurve easeInCurve;
        QEasingCurve easeOutCurve;
        double easeInDuration = 0;
        double easeOutDuration = 0;
        double strength = 1.0;
        double defaultFalloff = 0;
        double startTime = 0;
        double duration = 0;
        Clip *facade;
    };
}




#endif // CLIP_P_H
