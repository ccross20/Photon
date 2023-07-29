#ifndef PHOTON_NUMBERCHANNELPARAMETERVIEW_H
#define PHOTON_NUMBERCHANNELPARAMETERVIEW_H
#include "channelparameterview.h"

namespace photon {

class NumberChannelParameter;

class PHOTONCORE_EXPORT NumberChannelParameterView : public ChannelParameterView
{
public:
    NumberChannelParameterView(NumberChannelParameter *);
    ~NumberChannelParameterView();

private slots:
    void spinUpdated(double);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_NUMBERCHANNELPARAMETERVIEW_H
