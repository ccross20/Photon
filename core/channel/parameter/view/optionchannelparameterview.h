#ifndef PHOTON_OPTIONCHANNELPARAMETERVIEW_H
#define PHOTON_OPTIONCHANNELPARAMETERVIEW_H

#include "channelparameterview.h"

namespace photon {

class OptionChannelParameter;

class PHOTONCORE_EXPORT OptionChannelParameterView : public ChannelParameterView
{
public:
    OptionChannelParameterView(OptionChannelParameter *);
    ~OptionChannelParameterView();

private slots:
    void indexUpdated(int);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_OPTIONCHANNELPARAMETERVIEW_H
