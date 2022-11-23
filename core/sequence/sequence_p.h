#ifndef SEQUENCE_P_H
#define SEQUENCE_P_H

#include "sequence.h"

namespace photon
{

class Sequence::Impl
{
public:
    Impl(Sequence *);
    void addLayer(Layer*);
    void removeLayer(Layer*);
    QVector<Layer*> layers;
    QString name;
    Sequence *facade;
};

}

#endif // SEQUENCE_P_H
