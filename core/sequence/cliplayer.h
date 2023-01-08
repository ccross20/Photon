#ifndef PHOTON_CLIPLAYER_H
#define PHOTON_CLIPLAYER_H

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
