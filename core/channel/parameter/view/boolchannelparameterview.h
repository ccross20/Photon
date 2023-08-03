#ifndef PHOTON_BOOLCHANNELPARAMETERVIEW_H
#define PHOTON_BOOLCHANNELPARAMETERVIEW_H
#include "channelparameterview.h"

namespace photon {

class BoolChannelParameter;

class PHOTONCORE_EXPORT BoolChannelParameterView : public ChannelParameterView
{
public:
    BoolChannelParameterView(BoolChannelParameter *);
    ~BoolChannelParameterView();

private slots:
    void checkUpdated(bool);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_BOOLCHANNELPARAMETERVIEW_H
