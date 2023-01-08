#ifndef PHOTON_CANVASCOLLECTION_H
#define PHOTON_CANVASCOLLECTION_H

#include <QObject>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT CanvasCollection : public QObject
{
    Q_OBJECT
public:
    explicit CanvasCollection(QObject *parent = nullptr);
    ~CanvasCollection();

    int canvasCount() const;
    Canvas *canvasAtIndex(uint) const;
    const QVector<Canvas*> &canvases() const;

signals:
    void canvasWillBeAdded(photon::Canvas *, int);
    void canvasWasAdded(photon::Canvas *, int);
    void canvasWillBeRemoved(photon::Canvas *, int);
    void canvasWasRemoved(photon::Canvas *, int);

public slots:
    void addCanvas(photon::Canvas *);
    void removeCanvas(photon::Canvas *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CANVASCOLLECTION_H
