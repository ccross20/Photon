#ifndef CLIPEFFECT_P_H
#define CLIPEFFECT_P_H

#include "baseeffect.h"

namespace photon {

class BaseEffect::Impl
{
public:
    void processContext(BaseEffectEvaluationContext &, double);
    QVector<double> valuesForChannel(const QByteArray &, double);
    QColor colorForChannel(ChannelParameter *, double);
    QVector<Channel*> channels;
    ChannelParameterContainer *parameters;
    QString name;
    QByteArray id;
    QByteArray uniqueId;

    BaseEffectParent *effectParent = nullptr;
};

}

#endif // CLIPEFFECT_P_H
