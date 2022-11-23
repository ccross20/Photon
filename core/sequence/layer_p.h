#ifndef LAYER_P_H
#define LAYER_P_H

#include "layer.h"

namespace photon
{

class Layer::Impl
{
public:
    Impl(Layer *);
    void addClip(Clip *);
    void removeClip(Clip *);
    void setSequence(Sequence *t_sequence);
    void notifyClipWasModified(Clip *);
    QVector<Clip *> clips;
    QString name;
    Sequence *sequence = nullptr;
    Layer *facade;
};

}

#endif // LAYER_P_H
