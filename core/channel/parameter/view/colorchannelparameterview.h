#ifndef PHOTON_COLORCHANNELPARAMETERVIEW_H
#define PHOTON_COLORCHANNELPARAMETERVIEW_H

#include "channelparameterview.h"

namespace photon {


class ColorChannelParameter;

class PHOTONCORE_EXPORT ColorChannelParameterView : public ChannelParameterView
{
public:
    ColorChannelParameterView(ColorChannelParameter *);
    ~ColorChannelParameterView();

private slots:
    void colorUpdated(const QColor &);

private:
    class Impl;
    Impl *m_impl;
};


} // namespace photon

#endif // PHOTON_COLORCHANNELPARAMETERVIEW_H
