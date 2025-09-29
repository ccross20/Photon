#ifndef SURFACEGIZMOCONTAINER_H
#define SURFACEGIZMOCONTAINER_H

#include <QObject>
#include "photon-global.h"
#include "processcontext.h"

namespace photon {

class PHOTONCORE_EXPORT SurfaceGizmoContainer : public QObject
{
    Q_OBJECT
public:
    SurfaceGizmoContainer(QObject *parent = nullptr);
    ~SurfaceGizmoContainer();

    SurfaceGraph *graph() const;

    void processChannels(ProcessContext &, double lastTime);

    SurfaceGizmoContainerWidget *createWidget(SurfaceMode mode);

    const QVector<SurfaceGizmo*> &gizmos() const;

signals:
    void gizmoWillBeAdded(photon::SurfaceGizmo *, int);
    void gizmoWasAdded(photon::SurfaceGizmo *, int);
    void gizmoWillBeRemoved(photon::SurfaceGizmo *, int);
    void gizmoWasRemoved(photon::SurfaceGizmo *, int);


public slots:
    void addGizmo(photon::SurfaceGizmo *);
    void removeGizmo(photon::SurfaceGizmo *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // SURFACEGIZMOCONTAINER_H
