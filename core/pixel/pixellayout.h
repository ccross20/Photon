#ifndef PHOTON_PIXELLAYOUT_H
#define PHOTON_PIXELLAYOUT_H
#include "photon-global.h"
#include "pixel/canvas.h"

namespace photon {


class PHOTONCORE_EXPORT PixelLayout : public QObject
{
    Q_OBJECT
public:

    PixelLayout();
    virtual ~PixelLayout();

    QByteArray uniqueId() const;


    QString name() const;
    void setName(const QString &);

    void addSource(PixelSourceLayout *);
    void removeSource(PixelSourceLayout *);

    const QVector<PixelSourceLayout*> &sourceLayouts() const;

    QVector<PixelSource*> sources() const;

    virtual void process(ProcessContext &, double blend=1.0) const;

    virtual void readFromJson(const QJsonObject &, const LoadContext &);
    virtual void writeToJson(QJsonObject &) const;

signals:
    void sourceWillBeAdded(photon::PixelSourceLayout *, int);
    void sourceWasAdded(photon::PixelSourceLayout *, int);
    void sourceWillBeRemoved(photon::PixelSourceLayout *, int);
    void sourceWasRemoved(photon::PixelSourceLayout *, int);
    void layoutModified();

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PIXELLAYOUT_H
