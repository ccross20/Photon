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

    const QVector<SurfaceAction*> &actions() const;

    void processChannels(ProcessContext &, double lastTime);

    SurfaceGizmoContainerWidget *createWidget(SurfaceMode mode);

    const QVector<SurfaceGizmo*> &gizmos() const;
    QVector<SurfaceGizmo*> gizmosByType(const QByteArray &) const;
    SurfaceGizmo *findGizmoWithId(const QByteArray &) const;
    SurfaceGizmo *findGizmoWithUniqueId(const QByteArray &) const;

    virtual void restore(Project &);
    virtual void readFromJson(const QJsonObject &, const LoadContext &);
    virtual void writeToJson(QJsonObject &) const;

signals:
    void gizmoWillBeAdded(photon::SurfaceGizmo *, int);
    void gizmoWasAdded(photon::SurfaceGizmo *, int);
    void gizmoWillBeRemoved(photon::SurfaceGizmo *, int);
    void gizmoWasRemoved(photon::SurfaceGizmo *, int);
    void actionWillBeAdded(photon::SurfaceAction *, int);
    void actionWasAdded(photon::SurfaceAction *, int);
    void actionWillBeRemoved(photon::SurfaceAction *, int);
    void actionWasRemoved(photon::SurfaceAction *, int);


public slots:
    void addGizmo(photon::SurfaceGizmo *);
    void removeGizmo(photon::SurfaceGizmo *);
    void addAction(photon::SurfaceAction *);
    void removeAction(photon::SurfaceAction *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // SURFACEGIZMOCONTAINER_H
