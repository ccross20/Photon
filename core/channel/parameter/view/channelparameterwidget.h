#ifndef PHOTON_CHANNELPARAMETERWIDGET_H
#define PHOTON_CHANNELPARAMETERWIDGET_H

#include <QWidget>
#include "photon-global.h"
#include "sequence/channel.h"

namespace photon {

class ChannelParameterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ChannelParameterWidget(QVector<ChannelParameter*> parameters, ClipEffect *effect, QWidget *parent = nullptr);

signals:
    void addChannel(photon::ChannelParameter*, ChannelInfo::ChannelType);
    void removeChannel(photon::ChannelParameter*);
};

} // namespace photon

#endif // PHOTON_CHANNELPARAMETERWIDGET_H
