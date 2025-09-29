#ifndef SURFACE_H
#define SURFACE_H
#include "photon-global.h"
#include "processcontext.h"


namespace photon {

class PHOTONCORE_EXPORT Surface : public QObject
{
    Q_OBJECT
public:
    Surface(const QString &t_name = QString{}, QObject *parent = nullptr);
    ~Surface();
    void init();
    QString name() const;
    void setName(const QString &);
    Project *project() const;

    void processChannels(ProcessContext &, double lastTime);

    void restore(Project &);
    void readFromJson(const QJsonObject &, const LoadContext &);
    void writeToJson(QJsonObject &) const;

    const QVector<SurfaceGizmoContainer*> &containers() const;

signals:
    void gizmoContainerWillBeAdded(photon::SurfaceGizmoContainer *, int);
    void gizmoContainerWasAdded(photon::SurfaceGizmoContainer *, int);
    void gizmoContainerWillBeRemoved(photon::SurfaceGizmoContainer *, int);
    void gizmoContainerWasRemoved(photon::SurfaceGizmoContainer *, int);
    void surfaceWasModified();


public slots:
    void addGizmoContainer(photon::SurfaceGizmoContainer *);
    void removeGizmoContainer(photon::SurfaceGizmoContainer *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // SURFACE_H
