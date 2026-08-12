#ifndef PHOTON_PROJECTMODELTEST_H
#define PHOTON_PROJECTMODELTEST_H

#include <QObject>

namespace photon {

// Covers ProjectModel: the folder layout, the Rig branch mapping onto the live
// scene graph, live row updates, and the drag/drop rules.
class ProjectModelTest : public QObject
{
    Q_OBJECT
public:
    explicit ProjectModelTest(QObject *parent = nullptr);

private slots:
    void topLevelRowsAreTheFolders();
    void rigFolderExposesTheSceneHierarchy();
    void indexAndParentRoundTrip();
    void addingAResourceInsertsARow();
    void removingASceneObjectRemovesItsRow();
    void nestedSceneChangesReachTheModel();
    void detachedSceneObjectsDoNotEmitRowOps();
    void renamingAResourceEmitsDataChanged();
    void onlySceneObjectsAreDraggable();
    void dropsAreRejectedOutsideTheRig();
    void dropIntoOwnSubtreeIsRejected();

    void tagDropOntoAResourceAddsIt();
    void tagDropOntoAFolderIsRejected();
};

} // namespace photon

#endif // PHOTON_PROJECTMODELTEST_H
