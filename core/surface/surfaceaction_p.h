#ifndef SURFACEACTION_P_H
#define SURFACEACTION_P_H


#include "surfaceaction.h"

namespace photon
{
class SurfaceAction::Impl
{
public:
    Impl(SurfaceAction *);

    QVector<Channel*> channels;
    QVector<BaseEffect*> clipEffects;
    ChannelParameterContainer *parameters;
    QByteArray type;
    QByteArray uniqueId;
    QString name = "Action";
    QByteArray id;
    QByteArray enableOnGizmo;
    double strength = 1.0;
    double duration = 0.0;
    SurfaceAction *facade;
};

}

#endif // SURFACEACTION_P_H
