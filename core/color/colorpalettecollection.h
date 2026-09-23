#ifndef PHOTON_COLORPALETTECOLLECTION_H
#define PHOTON_COLORPALETTECOLLECTION_H

#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT ColorPaletteCollection : public QObject
{
    Q_OBJECT
public:
    ColorPaletteCollection(QObject *parent = nullptr);
    ~ColorPaletteCollection();

    int paletteCount() const;
    ColorPaletteResource *paletteAtIndex(uint) const;
    ColorPaletteResource *findPaletteWithId(const QByteArray &) const;
    ColorPaletteResource *findPaletteWithName(const QString &) const;
    const QVector<ColorPaletteResource*> &palettes() const;

signals:
    void paletteWillBeAdded(photon::ColorPaletteResource *, int);
    void paletteWasAdded(photon::ColorPaletteResource *, int);
    void paletteWillBeRemoved(photon::ColorPaletteResource *, int);
    void paletteWasRemoved(photon::ColorPaletteResource *, int);

public slots:
    void addPalette(photon::ColorPaletteResource *);
    void removePalette(photon::ColorPaletteResource *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_COLORPALETTECOLLECTION_H
