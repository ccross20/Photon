#ifndef SURFACECOLLECTION_H
#define SURFACECOLLECTION_H
#include <QObject>
#include "photon-global.h"

namespace photon {
class SurfacePanel;

class PHOTONCORE_EXPORT SurfaceCollection : public QObject
{
    Q_OBJECT
public:
    explicit SurfaceCollection(QObject *parent = nullptr);
    ~SurfaceCollection();

    void clear();
    int surfaceCount() const;
    Surface *surfaceAtIndex(uint) const;
    const QVector<Surface*> &surfaces() const;

    void editSurface(Surface *);
    void setActiveSurfacePanel(SurfacePanel *panel);
    SurfacePanel *activeSurfacePanel() const;
    Surface *activeSurface() const;

signals:
    void surfaceWillBeAdded(photon::Surface *, int);
    void surfaceWasAdded(photon::Surface *, int);
    void surfaceWillBeRemoved(photon::Surface *, int);
    void surfaceWasRemoved(photon::Surface *, int);

public slots:
    void addSurface(photon::Surface *);
    void removeSurface(photon::Surface *);
    void panelDestroyed(QObject*);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // SURFACECOLLECTION_H
