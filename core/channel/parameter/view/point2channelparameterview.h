#ifndef PHOTON_POINT2CHANNELPARAMETERVIEW_H
#define PHOTON_POINT2CHANNELPARAMETERVIEW_H

#include "channelparameterview.h"

namespace photon {

class Point2ChannelParameter;

class PHOTONCORE_EXPORT Point2ChannelParameterView : public ChannelParameterView
{
public:
    Point2ChannelParameterView(Point2ChannelParameter *);
    ~Point2ChannelParameterView();

private slots:
    void spinXUpdated(double);
    void spinYUpdated(double);

private:
    class Impl;
    Impl *m_impl;
};



} // namespace photon

#endif // PHOTON_POINT2CHANNELPARAMETERVIEW_H
