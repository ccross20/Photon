#ifndef PHOTON_COLORCOLLECTION_H
#define PHOTON_COLORCOLLECTION_H

#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT ColorCollection : public QObject
{
    Q_OBJECT
public:
    ColorCollection(QObject *parent = nullptr);
    ~ColorCollection();

    int colorCount() const;
    ColorResource *colorAtIndex(uint) const;
    ColorResource *findColorWithId(const QByteArray &) const;
    ColorResource *findColorWithName(const QString &) const;
    const QVector<ColorResource*> &colors() const;

signals:
    void colorWillBeAdded(photon::ColorResource *, int);
    void colorWasAdded(photon::ColorResource *, int);
    void colorWillBeRemoved(photon::ColorResource *, int);
    void colorWasRemoved(photon::ColorResource *, int);

public slots:
    void addColor(photon::ColorResource *);
    void removeColor(photon::ColorResource *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_COLORCOLLECTION_H
