#ifndef PHOTON_LAYER_H
#define PHOTON_LAYER_H

#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT Layer : public QObject
{
    Q_OBJECT
public:
    explicit Layer(const QString &name, const QByteArray &layerType, QObject *parent = nullptr);
    virtual ~Layer();

    QByteArray uniqueId() const;
    Sequence *sequence() const;
    virtual int height() const;
    QString name() const;
    void setName(const QString &name);
    QUuid guid() const;
    QByteArray layerType() const;


    virtual bool isGroup() const{return false;}
    virtual Layer *findLayerByGuid(const QUuid &guid);
    virtual void processChannels(ProcessContext &);
    virtual void restore(Project &);
    virtual void readFromJson(const QJsonObject &, const LoadContext &);
    virtual void writeToJson(QJsonObject &) const;

signals:
    void metadataChanged();

protected:
    virtual void sequenceChanged(Sequence *);

private:
    friend class Sequence;
    friend class LayerGroup;

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_LAYER_H
