#ifndef PHOTON_CHANNEL_H
#define PHOTON_CHANNEL_H

#include "photon-global.h"

namespace photon {


struct PHOTONCORE_EXPORT ChannelInfo
{
    enum ChannelType
    {
        ChannelTypeNumber,
        ChannelTypeColor
    };

    QString name;
    QString description;
    QVariant defaultValue;
    ChannelType type;
};

class PHOTONCORE_EXPORT Channel : public QObject
{
    Q_OBJECT
public:
    Channel(const ChannelInfo &info, double startTime, double duration, QObject *parent = nullptr);
    ~Channel();

    ChannelInfo info() const;
    void updateInfo(const ChannelInfo &);
    void addEffect(ChannelEffect *);
    void removeEffect(ChannelEffect *);
    void moveEffect(ChannelEffect *, int newPosition);

    double processDouble(double time);

    void setDuration(double);
    double duration() const;

    void setStartTime(double);
    double startTime() const;

    void setEndTime(double);
    double endTime() const;

    int effectCount() const;
    ChannelEffect *effectAtIndex(int index) const;

    Sequence *sequence() const;

    void effectUpdated(ChannelEffect *);

    void restore(Project &);
    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

signals:

    void effectModified(photon::ChannelEffect *);
    void effectAdded(photon::ChannelEffect*);
    void effectRemoved(photon::ChannelEffect*);
    void effectMoved(photon::ChannelEffect*);
    void channelUpdated(photon::Channel*);
private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CHANNEL_H
