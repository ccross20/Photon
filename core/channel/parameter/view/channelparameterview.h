#ifndef PHOTON_CHANNELPARAMETERVIEW_H
#define PHOTON_CHANNELPARAMETERVIEW_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT ChannelParameterView : public QWidget
{
    Q_OBJECT
public:
    ChannelParameterView(ChannelParameter *);
    virtual ~ChannelParameterView();

    ChannelParameter *parameter() const;

signals:
    void parameterUpdated();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CHANNELPARAMETERVIEW_H
