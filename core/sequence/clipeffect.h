#ifndef PHOTON_CLIPEFFECT_H
#define PHOTON_CLIPEFFECT_H

#include "photon-global.h"
#include "channel.h"

namespace photon {

struct ClipEffectInformation
{

    ClipEffectInformation(){}
    ClipEffectInformation(std::function<ClipEffect*()> _callback):callback(_callback){}

    QByteArray id;
    QByteArray translateId;
    QByteArray iconId;
    QString name;
    bool isCanvas = false;
    std::function<ClipEffect*()> callback;

    CategoryList categories;
};

struct ClipEffectEvaluationContext
{
    ClipEffectEvaluationContext(DMXMatrix &matrix):dmxMatrix(matrix){}
    DMXMatrix &dmxMatrix;
    Project *project = nullptr;
    Fixture *fixture = nullptr;
    QImage *canvasImage = nullptr;
    QImage *previousCanvasImage = nullptr;
    QHash<QByteArray,QVariant> channelValues;
    double relativeTime = 0.0;
    double globalTime = 0.0;
    double delayTime = 0.0;
    double strength = 1.0;
    int fixtureIndex = 0;
    int fixtureTotal = 0;
};

class PHOTONCORE_EXPORT ClipEffect : public QObject
{
    Q_OBJECT
public:
    ClipEffect(const QByteArray &t_id = QByteArray());
    virtual ~ClipEffect();

    virtual void init();
    virtual void processChannels(ProcessContext &) const;
    virtual void evaluateFixture(ClipEffectEvaluationContext &) const;

    QByteArray id() const;
    QByteArray uniqueId() const;
    void setId(const QByteArray &);
    void setName(const QString &);
    QString name() const;
    void updated();
    virtual QWidget *createEditor();

    double getNumberAtTime(const QByteArray &id, double) const;
    QColor getColorAtTime(const QByteArray &id, double) const;

    Channel *channelAtIndex(int index) const;
    int channelCount() const;
    void clearChannels();
    const QVector<Channel*> channels() const;

    Clip *clip() const;
    virtual void restore(Project &);
    virtual void readFromJson(const QJsonObject &, const LoadContext &);
    virtual void writeToJson(QJsonObject &) const;

protected:
    virtual void startTimeUpdated(double);
    virtual void durationUpdated(double);

public slots:
    photon::Channel *addChannel(const photon::ChannelInfo &info = ChannelInfo{}, int index = -1);
    void removeChannel(int index);
    void channelUpdatedSlot(photon::Channel *);

signals:

    void channelUpdated(photon::Channel *);
    void channelAdded(photon::Channel *);
    void channelRemoved(photon::Channel *);
    void channelMoved(photon::Channel *);

private:
    friend class Clip;
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CLIPEFFECT_H
