#ifndef PHOTON_SURFACEOWNERSHIPTEST_H
#define PHOTON_SURFACEOWNERSHIPTEST_H

#include <QObject>

namespace photon {

// Covers the ownership inversion: the Project owns Surfaces and SurfaceNodes
// only reference one by id, so several nodes can drive the same surface and a
// surface outlives any node pointing at it.
class SurfaceOwnershipTest : public QObject
{
    Q_OBJECT
public:
    explicit SurfaceOwnershipTest(QObject *parent = nullptr);

private slots:
    void newProjectSeedsOneSurfaceBoundToTheBusNode();
    void collectionRevisionTracksMembership();
    void removedSurfaceStopsResolving();
    void nodeSerializesOnlyItsSurfaceId();
    void projectWritesSurfacesAtTopLevel();
    void readReplacesSeededSurfaces();
};

} // namespace photon

#endif // PHOTON_SURFACEOWNERSHIPTEST_H
