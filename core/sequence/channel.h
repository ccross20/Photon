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
    Channel(Clip *clip, const ChannelInfo &info);
    ~Channel();

    ChannelInfo info() const;
    void addEffect(ChannelEffect *);
    void removeEffect(ChannelEffect *);
    void moveEffect(ChannelEffect *, int newPosition);

    double processDouble(double time);
    Clip *clip() const;

    int effectCount() const;
    ChannelEffect *effectAtIndex(int index) const;

    void effectUpdated(ChannelEffect *);

    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

signals:

    void effectModified(photon::ChannelEffect *);
    void effectAdded(photon::ChannelEffect*);
    void effectRemoved(photon::ChannelEffect*);
    void effectMoved(photon::ChannelEffect*);

private:
    friend class Clip;
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CHANNEL_H
