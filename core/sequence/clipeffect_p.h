#ifndef CLIPEFFECT_P_H
#define CLIPEFFECT_P_H

#include "clipeffect.h"

namespace photon {

class ClipEffect::Impl
{
public:
    void processContext(ClipEffectEvaluationContext &, double);
    QVector<double> valuesForChannel(const QByteArray &, double);
    QColor colorForChannel(ChannelParameter *, double);
    QVector<Channel*> channels;
    ChannelParameterContainer *parameters;
    QString name;
    QByteArray id;
    QByteArray uniqueId;
    Clip *clip = nullptr;
};

}

#endif // CLIPEFFECT_P_H
