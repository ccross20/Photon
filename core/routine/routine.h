#ifndef PHOTON_ROUTINE_H
#define PHOTON_ROUTINE_H

#include "model/graph.h"
#include "photon-global.h"
#include "sequence/channel.h"

namespace photon {

class PHOTONCORE_EXPORT Routine : public keira::Graph
{
    Q_OBJECT
public:



    explicit Routine(const QString &t_name = QString{}, QObject *parent = nullptr);
    ~Routine();

    QString name() const;
    void setName(const QString &);

    void addChannel(const ChannelInfo &info);
    void removeChannel(int index);
    int channelCount() const;
    ChannelInfo channelInfoAtIndex(int index);
    const QVector<ChannelInfo> &channelInfo() const;
    void evaluate(keira::EvaluationContext *) const override;


    void readFromJson(const QJsonObject &, keira::NodeLibrary *library) override;
    void writeToJson(QJsonObject &) const override;

signals:
    void channelAdded(int index);
    void channelRemoved(int index);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_ROUTINE_H
