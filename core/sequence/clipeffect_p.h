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
    ChannelParameter *findChannelParameter(const QByteArray &);
    QVector<Channel*> channels;
    QVector<ChannelParameter*> channelParameters;
    QString name;
    QByteArray id;
    QByteArray uniqueId;
    Clip *clip = nullptr;
};

}

#endif // CLIPEFFECT_P_H
