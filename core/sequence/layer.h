#ifndef PHOTON_LAYER_H
#define PHOTON_LAYER_H

#include <QColor>
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

    // The colour its clips show when they have none of their own - one per
    // layer, picked by the layer's position in the sequence so neighbouring
    // layers stand apart.
    QColor defaultClipColor() const;

    // Muted layers are skipped entirely by Sequence::processChannels - their
    // clips keep their data (nothing is deleted), they just stop contributing
    // to output until unmuted.
    bool isMuted() const;
    void setMuted(bool muted);

    virtual QWidget *createEditor();
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

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_LAYER_H
