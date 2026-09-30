#ifndef PHOTON_CLIPLAYER_H
#define PHOTON_CLIPLAYER_H

#include <QJsonObject>
#include "layer.h"

namespace photon {

class PHOTONCORE_EXPORT ClipLayer : public Layer
{
    Q_OBJECT
public:
    ClipLayer(const QString &name = QString{}, QObject *parent = nullptr);
    ~ClipLayer();

    void addClip(Clip *);
    void removeClip(Clip *);
    // Adds an independent copy of the clip (content, timing, easing) to this
    // layer, with its own unique id. Null if the clip type can't be created.
    Clip *duplicateClip(const Clip *);
    // Adds a new clip built from saved clip JSON (Clip::writeToJson), with a
    // fresh unique id. Null if the clip type can't be created.
    Clip *addClipFromJson(QJsonObject clipJson);
    const QVector<Clip*> &clips() const;

    void processChannels(ProcessContext &) override;
    void restore(Project &) override;
    void readFromJson(const QJsonObject &, const LoadContext &) override;
    void writeToJson(QJsonObject &) const override;

protected:
    void sequenceChanged(Sequence *) override;

signals:

    void clipAdded(photon::Clip *);
    void clipRemoved(photon::Clip *);
    void clipModified(photon::Clip *);

private:
    friend class Clip;

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CLIPLAYER_H
