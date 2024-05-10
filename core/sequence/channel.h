#ifndef PHOTON_CHANNEL_H
#define PHOTON_CHANNEL_H

#include "photon-global.h"

namespace photon {


struct PHOTONCORE_EXPORT ChannelInfo
{
    enum ChannelType
    {
        ChannelTypeNumber,
        ChannelTypeInteger,
        ChannelTypeIntegerStep,
        ChannelTypeBool,
        ChannelTypeColor,
        ChannelTypePoint
    };

    ChannelInfo(ChannelType type = ChannelTypeNumber, const QString &name = QString{}, const QString &description = QString{}, const QVariant &defaultValue = QVariant{}):
    name(name),
      description(description),
      defaultValue(defaultValue),
      type(type)
    {
        uniqueId = QUuid::createUuid().toByteArray();
    }

    ChannelInfo(const QByteArray &id, ChannelType type = ChannelTypeNumber, const QString &name = QString{}, const QString &description = QString{}, const QVariant &defaultValue = QVariant{}):
    name(name),
      description(description),
      defaultValue(defaultValue),
      uniqueId(id),
      type(type)
    {

    }


    void readFromJson(const QJsonObject &);
    void writeToJson(QJsonObject &) const;

    QStringList subChannelNames;
    QStringList options;
    QString name;
    QString description;
    QString parentName;
    QVariant defaultValue;
    QByteArray uniqueId;
    QByteArray parentUniqueId;
    ChannelType type;
    int subChannelIndex = 0;
};

class PHOTONCORE_EXPORT Channel : public QObject
{
    Q_OBJECT
public:
    Channel(const ChannelInfo &info, double startTime, double duration, QObject *parent = nullptr);
    ~Channel();

    void addSubChannel(Channel *);
    void removeSubChannel(Channel *);
    const QVector<Channel*> &subChannels() const;
    int subChannelCount() const;

    QByteArray uniqueId() const;
    ChannelInfo info() const;
    ChannelInfo::ChannelType type() const;
    void updateInfo(const ChannelInfo &);
    void addEffect(ChannelEffect *);
    void removeEffect(ChannelEffect *);
    void moveEffect(ChannelEffect *, int newPosition);

    QVariant processValue(double time);

    void setDuration(double);
    double duration() const;

    void setStartTime(double);
    double startTime() const;

    void setEndTime(double);
    double endTime() const;

    void setParentName(const QString &);
    QString parentName() const;

    QByteArray parentUniqueId() const;
    int subChannelIndex() const;

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
