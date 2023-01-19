#ifndef ROUTINEEVALUATIONCONTEXT_H
#define ROUTINEEVALUATIONCONTEXT_H

#include "photon-global.h"
#include "data/dmxmatrix.h"
#include "model/node.h"
namespace photon {

struct RoutineEvaluationContext : keira::EvaluationContext
{
    RoutineEvaluationContext(DMXMatrix &matrix):dmxMatrix(matrix){}
    DMXMatrix &dmxMatrix;
    Fixture *fixture = nullptr;
    QImage *canvasImage = nullptr;
    QVector<QVariant> channelValues;
    double relativeTime = 0.0;
    double globalTime = 0.0;
    double delayTime = 0.0;
    double strength = 1.0;
};

}

#endif // ROUTINEEVALUATIONCONTEXT_H
