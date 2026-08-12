#include <QTest>
#include <QMimeData>
#include <QSignalSpy>
#include "projectmodeltest.h"
#include "project/project.h"
#include "project/projectmodel.h"
#include "scene/sceneobject.h"
#include "scene/scenemodel.h"
#include "fixture/fixturegroup.h"
#include "routine/routine.h"
#include "routine/routinecollection.h"
#include "gui/tag/tagmime.h"

namespace photon {

namespace {

// Builds "parent" and "child" under the scene root, returning both.
struct Branch
{
    SceneObject *parent;
    SceneObject *child;
};

Branch buildBranch(Project &project)
{
    auto *parent = new SceneObject("group");
    parent->setName("Truss A");
    parent->setParentSceneObject(project.sceneRoot());

    auto *child = new SceneObject("group");
    child->setName("Mover 1");
    child->setParentSceneObject(parent);

    return {parent, child};
}

} // namespace

ProjectModelTest::ProjectModelTest(QObject *parent)
    : QObject{parent}
{
}

void ProjectModelTest::topLevelRowsAreTheFolders()
{
    Project project;
    ProjectModel model(&project);

    QCOMPARE(model.rowCount(QModelIndex()), 6);
    QCOMPARE(model.columnCount(QModelIndex()), int(ProjectModel::ColumnCount));

    const QStringList expected{"Rig", "Fixture Groups", "Routines", "Sequences",
                               "Surfaces", "Pixel Layouts"};
    for(int row = 0; row < expected.size(); ++row)
    {
        const QModelIndex index = model.index(row, ProjectModel::NameColumn);
        QCOMPARE(index.data(Qt::DisplayRole).toString(), expected.at(row));
        QVERIFY(index.data(ProjectModel::IsFolderRole).toBool());
        // Folders are not renameable and not draggable.
        QVERIFY(!(model.flags(index) & Qt::ItemIsEditable));
        QVERIFY(!(model.flags(index) & Qt::ItemIsDragEnabled));
    }
}

void ProjectModelTest::rigFolderExposesTheSceneHierarchy()
{
    Project project;
    const Branch branch = buildBranch(project);

    ProjectModel model(&project);

    const QModelIndex rigIndex = model.index(0, ProjectModel::NameColumn);
    QCOMPARE(rigIndex.data(Qt::DisplayRole).toString(), QStringLiteral("Rig"));

    // The scene root is not a row of its own; its children hang directly off
    // the Rig folder.
    QCOMPARE(model.rowCount(rigIndex), 1);

    const QModelIndex parentIndex = model.index(0, ProjectModel::NameColumn, rigIndex);
    QCOMPARE(parentIndex.data(Qt::DisplayRole).toString(), QStringLiteral("Truss A"));
    QCOMPARE(model.resourceForIndex(parentIndex), static_cast<ProjectResource*>(branch.parent));

    // ...and the nesting continues down the real hierarchy.
    QCOMPARE(model.rowCount(parentIndex), 1);
    const QModelIndex childIndex = model.index(0, ProjectModel::NameColumn, parentIndex);
    QCOMPARE(childIndex.data(Qt::DisplayRole).toString(), QStringLiteral("Mover 1"));
    QCOMPARE(model.resourceForIndex(childIndex), static_cast<ProjectResource*>(branch.child));
}

void ProjectModelTest::indexAndParentRoundTrip()
{
    Project project;
    const Branch branch = buildBranch(project);
    auto *group = project.groups()->addGroup("Front");

    ProjectModel model(&project);

    // A nested scene object walks back up through its real parent.
    const QModelIndex childIndex = model.indexForResource(branch.child);
    QVERIFY(childIndex.isValid());
    QCOMPARE(model.parent(childIndex), model.indexForResource(branch.parent));

    // A top-level scene object's parent is the Rig folder, not an invalid index.
    const QModelIndex parentIndex = model.indexForResource(branch.parent);
    QCOMPARE(model.parent(parentIndex), model.index(0, 0));

    // A flat resource's parent is its type's folder.
    const QModelIndex groupIndex = model.indexForResource(group);
    QVERIFY(groupIndex.isValid());
    QCOMPARE(model.parent(groupIndex), model.indexForResource(model.folderForContentType("group")));

    // Folders sit at the root.
    QVERIFY(!model.parent(model.index(0, 0)).isValid());
}

void ProjectModelTest::addingAResourceInsertsARow()
{
    Project project;
    ProjectModel model(&project);

    const QModelIndex routineFolder = model.indexForResource(model.folderForContentType("routine"));
    QCOMPARE(model.rowCount(routineFolder), 0);

    QSignalSpy insertSpy(&model, &QAbstractItemModel::rowsInserted);

    project.routines()->addRoutine(new Routine("Chase"));

    // Inserted as a row, not via a full model reset - a reset would drop the
    // view's selection and expansion state.
    QCOMPARE(insertSpy.count(), 1);
    QCOMPARE(model.rowCount(routineFolder), 1);
    QCOMPARE(model.index(0, ProjectModel::NameColumn, routineFolder).data().toString(),
             QStringLiteral("Chase"));
}

void ProjectModelTest::removingASceneObjectRemovesItsRow()
{
    Project project;
    const Branch branch = buildBranch(project);
    ProjectModel model(&project);

    const QModelIndex parentIndex = model.indexForResource(branch.parent);
    QCOMPARE(model.rowCount(parentIndex), 1);

    QSignalSpy removeSpy(&model, &QAbstractItemModel::rowsRemoved);

    branch.child->setParentSceneObject(nullptr);

    QCOMPARE(removeSpy.count(), 1);
    QCOMPARE(model.rowCount(model.indexForResource(branch.parent)), 0);
}

void ProjectModelTest::nestedSceneChangesReachTheModel()
{
    Project project;
    const Branch branch = buildBranch(project);
    ProjectModel model(&project);

    QSignalSpy insertSpy(&model, &QAbstractItemModel::rowsInserted);

    // Only the descendant* signals bubble up the scene graph - child* ones are
    // emitted by the immediate parent alone. Adding two levels down must still
    // reach the model, which means every object has to be watched.
    auto *grandchild = new SceneObject("group");
    grandchild->setName("Head");
    grandchild->setParentSceneObject(branch.child);

    QCOMPARE(insertSpy.count(), 1);
    QCOMPARE(model.rowCount(model.indexForResource(branch.child)), 1);
    QCOMPARE(model.indexForResource(grandchild).data().toString(), QStringLiteral("Head"));

    // ...including a branch that arrives already populated, whose descendants
    // must be watched too.
    auto *newBranch = new SceneObject("group");
    auto *newBranchChild = new SceneObject("group");
    newBranchChild->setParentSceneObject(newBranch);
    newBranch->setParentSceneObject(project.sceneRoot());

    insertSpy.clear();
    auto *deepest = new SceneObject("group");
    deepest->setParentSceneObject(newBranchChild);
    QCOMPARE(insertSpy.count(), 1);
}

void ProjectModelTest::detachedSceneObjectsDoNotEmitRowOps()
{
    Project project;
    const Branch branch = buildBranch(project);
    ProjectModel model(&project);

    // Detach a watched branch. It stays alive and still connected, so further
    // edits to it must not be reported as row operations against a tree it is
    // no longer part of.
    branch.parent->setParentSceneObject(nullptr);

    QSignalSpy insertSpy(&model, &QAbstractItemModel::rowsInserted);
    QSignalSpy removeSpy(&model, &QAbstractItemModel::rowsRemoved);

    auto *orphan = new SceneObject("group");
    orphan->setParentSceneObject(branch.child);
    QCOMPARE(insertSpy.count(), 0);

    orphan->setParentSceneObject(nullptr);
    QCOMPARE(removeSpy.count(), 0);

    delete branch.parent;
}

void ProjectModelTest::renamingAResourceEmitsDataChanged()
{
    Project project;
    ProjectModel model(&project);

    auto *routine = new Routine("Chase");
    project.routines()->addRoutine(routine);

    QSignalSpy changedSpy(&model, &QAbstractItemModel::dataChanged);

    // Renaming through the resource contract must reach the view.
    routine->setName("Chase 2");
    QCOMPARE(changedSpy.count(), 1);

    // ...and so must a tag edit, since tags are a visible column.
    routine->addResourceTag("act1");
    QCOMPARE(changedSpy.count(), 2);

    const QModelIndex routineFolder = model.indexForResource(model.folderForContentType("routine"));
    QCOMPARE(model.index(0, ProjectModel::TagsColumn, routineFolder).data().toString(),
             QStringLiteral("act1"));
}

void ProjectModelTest::onlySceneObjectsAreDraggable()
{
    Project project;
    const Branch branch = buildBranch(project);
    auto *group = project.groups()->addGroup("Front");

    ProjectModel model(&project);

    QVERIFY(model.flags(model.indexForResource(branch.parent)) & Qt::ItemIsDragEnabled);
    // Flat resources have no hierarchy to rearrange.
    QVERIFY(!(model.flags(model.indexForResource(group)) & Qt::ItemIsDragEnabled));
}

void ProjectModelTest::dropsAreRejectedOutsideTheRig()
{
    Project project;
    const Branch branch = buildBranch(project);
    project.routines()->addRoutine(new Routine("Chase"));

    ProjectModel model(&project);

    QScopedPointer<QMimeData> mime(encodeSceneObjectMime({branch.child}));

    // Onto another scene object: fine.
    QVERIFY(model.canDropMimeData(mime.data(), Qt::MoveAction, -1, -1,
                                  model.indexForResource(branch.parent)));

    // Onto the Rig folder: reparents to the scene root.
    QVERIFY(model.canDropMimeData(mime.data(), Qt::MoveAction, -1, -1,
                                  model.indexForResource(model.folderForContentType("scene"))));

    // Onto the Routines folder, or onto a routine: rejected.
    auto *routineFolder = model.folderForContentType("routine");
    QVERIFY(!model.canDropMimeData(mime.data(), Qt::MoveAction, -1, -1,
                                   model.indexForResource(routineFolder)));
    QVERIFY(!model.canDropMimeData(mime.data(), Qt::MoveAction, -1, -1,
                                   model.index(0, 0, model.indexForResource(routineFolder))));
}

void ProjectModelTest::dropIntoOwnSubtreeIsRejected()
{
    Project project;
    const Branch branch = buildBranch(project);
    ProjectModel model(&project);

    // Dragging a parent onto its own descendant would detach the branch from
    // the tree entirely.
    QScopedPointer<QMimeData> mime(encodeSceneObjectMime({branch.parent}));

    QVERIFY(!model.canDropMimeData(mime.data(), Qt::MoveAction, -1, -1,
                                   model.indexForResource(branch.child)));
    // Onto itself, likewise.
    QVERIFY(!model.canDropMimeData(mime.data(), Qt::MoveAction, -1, -1,
                                   model.indexForResource(branch.parent)));
}

void ProjectModelTest::tagDropOntoAResourceAddsIt()
{
    Project project;
    project.routines()->addRoutine(new Routine("Chase"));
    ProjectModel model(&project);

    const QModelIndex routineIndex = model.index(0, 0, model.indexForResource(model.folderForContentType("routine")));
    QVERIFY(routineIndex.isValid());

    QScopedPointer<QMimeData> mime(encodeTagMime({"act1"}));

    QVERIFY(model.canDropMimeData(mime.data(), Qt::CopyAction, -1, -1, routineIndex));
    QVERIFY(model.dropMimeData(mime.data(), Qt::CopyAction, -1, -1, routineIndex));

    QVERIFY(model.resourceForIndex(routineIndex)->resourceTags().contains("act1"));
}

void ProjectModelTest::tagDropOntoAFolderIsRejected()
{
    Project project;
    ProjectModel model(&project);

    QScopedPointer<QMimeData> mime(encodeTagMime({"act1"}));

    // Nothing to tag on a folder row itself.
    QVERIFY(!model.canDropMimeData(mime.data(), Qt::CopyAction, -1, -1,
                                   model.indexForResource(model.folderForContentType("routine"))));
}

} // namespace photon
