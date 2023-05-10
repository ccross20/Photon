#ifndef STATEEVALUATIONCONTEXT_H
#define STATEEVALUATIONCONTEXT_H

#include "photon-global.h"
#include "data/dmxmatrix.h"
#include "model/node.h"
namespace photon {

struct StateEvaluationContext : keira::EvaluationContext
{
    StateEvaluationContext(DMXMatrix &matrix):dmxMatrix(matrix){}
    DMXMatrix &dmxMatrix;
    Fixture *fixture = nullptr;
    QHash<QByteArray,QVariant> channelValues;
    double relativeTime = 0.0;
    double globalTime = 0.0;
    double delayTime = 0.0;
    double strength = 1.0;
};

}

#endif // STATEEVALUATIONCONTEXT_H
