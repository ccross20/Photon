#ifndef LAYER_P_H
#define LAYER_P_H

#include "layer.h"

namespace photon
{

class Layer::Impl
{
public:
    Impl(Layer *);
    void setSequence(Sequence *t_sequence);
    QString name;
    QByteArray type;
    QUuid guid;
    Sequence *sequence = nullptr;
    Layer *facade;
};

}

#endif // LAYER_P_H
