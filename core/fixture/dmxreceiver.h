#ifndef DMXRECEIVER_H
#define DMXRECEIVER_H

#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT DMXReceiver
{
public:
    virtual ~DMXReceiver(){}
    virtual int dmxOffset() const = 0;
    virtual int dmxSize() const = 0;
    virtual int universe() const = 0;
};

}

#endif // DMXRECEIVER_H
