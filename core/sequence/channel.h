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
        ChannelTypeColor
    };

    ChannelInfo(ChannelType type = ChannelTypeNumber, const QString &name = QString{}, const QString &description = QString{}, const QVariant &defaultValue = QVariant{}):
    name(name),
      description(description),
      defaultValue(defaultValue),
      type(type)
    {
        uniqueId = QUuid::createUuid().toByteArray();
    }


    void readFromJson(const QJsonObject &);
    void writeToJson(QJsonObject &) const;

    QStringList options;
    QString name;
    QString description;
    QVariant defaultValue;
    QByteArray uniqueId;
    ChannelType type;
};

class PHOTONCORE_EXPORT Channel : public QObject
{
    Q_OBJECT
public:
    Channel(const ChannelInfo &info, double startTime, double duration, QObject *parent = nullptr);
    ~Channel();

    QByteArray uniqueId() const;
    ChannelInfo info() const;
    void updateInfo(const ChannelInfo &);
    void addEffect(ChannelEffect *);
    void removeEffect(ChannelEffect *);
    void moveEffect(ChannelEffect *, int newPosition);

    double processDouble(double time);
    QColor processColor(double time);

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
