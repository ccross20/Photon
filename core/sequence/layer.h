#ifndef PHOTON_LAYER_H
#define PHOTON_LAYER_H

#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT Layer : public QObject
{
    Q_OBJECT
public:
    explicit Layer(const QString &name = QString{}, QObject *parent = nullptr);
    ~Layer();

    Sequence *sequence() const;
    void addClip(Clip *);
    void removeClip(Clip *);
    int height() const;
    const QVector<Clip*> &clips() const;
    QString name() const;
    void setName(const QString &name);

    void processChannels(ProcessContext &);
    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

signals:

    void clipAdded(photon::Clip *);
    void clipRemoved(photon::Clip *);
    void clipModified(photon::Clip *);

private:
    friend class Sequence;
    friend class Clip;

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_LAYER_H
