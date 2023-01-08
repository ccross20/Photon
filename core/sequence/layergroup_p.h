#ifndef LAYERGROUP_P_H
#define LAYERGROUP_P_H

#include "layergroup.h"

namespace photon {

class LayerGroup::Impl
{
public:
    QVector<Layer*> layers;
};

}

#endif // LAYERGROUP_P_H
