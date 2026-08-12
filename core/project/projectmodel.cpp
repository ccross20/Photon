#include <QMimeData>
#include <QSize>
#include "projectmodel.h"
#include "project/project.h"
#include "photoncore.h"
#include "scene/sceneobject.h"
#include "scene/scenemodel.h"
#include "scene/scenemanager.h"
#include "fixture/fixturegroup.h"
#include "routine/routine.h"
#include "routine/routinecollection.h"
#include "sequence/sequence.h"
#include "sequence/sequencecollection.h"
#include "surface/surface.h"
#include "surface/surfacecollection.h"
#include "pixel/pixellayout.h"
#include "pixel/pixellayoutcollection.h"
#include "gui/tag/tagmime.h"

namespace photon {

namespace {
    // Content type of the Rig folder. Scene objects report their own concrete
    // typeId ("fixture", "truss", ...), so the folder needs a tag of its own.
    const QByteArray SceneContentType = "scene";
}

ProjectFolder::ProjectFolder(const QString &t_name, const QByteArray &t_contentTypeId, QObject *parent)
    : QObject{parent}, m_name(t_name), m_contentTypeId(t_contentTypeId)
{
}

QByteArray ProjectFolder::contentTypeId() const
{
    return m_contentTypeId;
}

QByteArray ProjectFolder::resourceId() const
{
    return "folder:" + m_contentTypeId;
}

class ProjectModel::Impl
{
public:
    ProjectModel *facade = nullptr;
    Project *project = nullptr;

    QList<ProjectFolder*> folders;
    ProjectFolder *rigFolder = nullptr;
    // Cached contents of the flat folders, kept in step with their collections.
    // The Rig folder is absent here: its children are read live from the scene
    // graph, which is a tree rather than a list.
    QHash<ProjectFolder*, QList<ProjectResource*>> flatChildren;

    SceneObject *sceneRoot() const{return project ? project->sceneRoot() : nullptr;}

    ProjectFolder *addFolder(const QString &name, const QByteArray &contentTypeId);
    ProjectFolder *folderForResource(ProjectResource *) const;

    // Live children of a folder, whichever way it stores them.
    QList<ProjectResource*> childrenOf(ProjectFolder *) const;
    int rowOfResource(ProjectResource *) const;

    // Re-read a flat folder's contents from its collection. The collections
    // disagree on signal signatures (some carry an index, some don't), so the
    // constructor wires each one explicitly and they all funnel through here.
    void refreshFlat(ProjectFolder *folder, std::function<QList<ProjectResource*>()> read);
    void watchResource(ProjectResource *);
    void resourceUpdated(ProjectResource *);

    // Connect one object's structural signals, then recurse into its children.
    void watchSceneSubtree(SceneObject *);
    // Whether an object is still attached to the scene root. A detached object
    // stays alive and connected, and its stale index() would produce a bogus
    // QModelIndex, so row operations are skipped for it.
    bool isInScene(SceneObject *) const;

    // Scene-graph relays.
    void childWillBeAdded(SceneObject *parent, SceneObject *child);
    void childWasAdded(SceneObject *child);
    void childWillBeRemoved(SceneObject *parent, SceneObject *child);
    void childWasRemoved(SceneObject *child);
    void childWillBeMoved(SceneObject *child, int newIndex);
    void childWasMoved(SceneObject *child);

    // Set when a begin*Rows() was issued, so the matching end*() is only called
    // if the begin actually happened.
    bool pendingInsert = false;
    bool pendingRemove = false;
    bool pendingMove = false;
};

ProjectFolder *ProjectModel::Impl::addFolder(const QString &t_name, const QByteArray &t_contentTypeId)
{
    auto *folder = new ProjectFolder(t_name, t_contentTypeId, facade);
    folders.append(folder);
    return folder;
}

QList<ProjectResource*> ProjectModel::Impl::childrenOf(ProjectFolder *t_folder) const
{
    if(t_folder == rigFolder)
    {
        QList<ProjectResource*> children;
        if(auto *root = sceneRoot())
            for(auto *child : root->sceneChildren())
                children.append(child);
        return children;
    }

    return flatChildren.value(t_folder);
}

ProjectFolder *ProjectModel::Impl::folderForResource(ProjectResource *t_resource) const
{
    if(!t_resource)
        return nullptr;

    if(dynamic_cast<SceneObject*>(t_resource))
        return rigFolder;

    for(auto *folder : folders)
        if(folder->contentTypeId() == t_resource->resourceTypeId())
            return folder;

    return nullptr;
}

int ProjectModel::Impl::rowOfResource(ProjectResource *t_resource) const
{
    if(auto *sceneObject = dynamic_cast<SceneObject*>(t_resource))
        return sceneObject->index();

    if(auto *folder = folderForResource(t_resource))
        return flatChildren.value(folder).indexOf(t_resource);

    return -1;
}

void ProjectModel::Impl::watchResource(ProjectResource *t_resource)
{
    if(!t_resource)
        return;

    QObject::connect(t_resource->resourceNotifier(), &ProjectResourceNotifier::resourceChanged,
                     facade, [this](ProjectResource *resource){ resourceUpdated(resource); });
}

void ProjectModel::Impl::resourceUpdated(ProjectResource *t_resource)
{
    const QModelIndex index = facade->indexForResource(t_resource);
    if(!index.isValid())
        return;

    emit facade->dataChanged(index, index.sibling(index.row(), ColumnCount - 1));
}

void ProjectModel::Impl::refreshFlat(ProjectFolder *t_folder, std::function<QList<ProjectResource*>()> t_read)
{
    const QList<ProjectResource*> updated = t_read();

    for(auto *resource : updated)
        if(!flatChildren[t_folder].contains(resource))
            watchResource(resource);

    flatChildren[t_folder] = updated;
}

ProjectModel::ProjectModel(Project *t_project, QObject *parent)
    : QAbstractItemModel{parent}, m_impl(new Impl)
{
    m_impl->facade = this;
    m_impl->project = t_project;

    m_impl->rigFolder = m_impl->addFolder("Rig", SceneContentType);
    auto *groupFolder    = m_impl->addFolder("Fixture Groups", "group");
    auto *routineFolder  = m_impl->addFolder("Routines", "routine");
    auto *sequenceFolder = m_impl->addFolder("Sequences", "sequence");
    auto *surfaceFolder  = m_impl->addFolder("Surfaces", "surface");
    auto *layoutFolder   = m_impl->addFolder("Pixel Layouts", "pixel-layout");

    // ---- Flat folders ------------------------------------------------------
    // Each collection exposes its own signal shapes, so rather than a generic
    // adapter each one gets a pair of lambdas: one to bracket the row op, one
    // to re-read the contents.

    // Fixture groups.
    {
        auto *collection = t_project->groups();
        auto read = [collection](){
            QList<ProjectResource*> list;
            for(auto *group : collection->groups())
                list.append(group);
            return list;
        };
        m_impl->refreshFlat(groupFolder, read);

        connect(collection, &FixtureGroupCollection::groupWillBeAddedAt, this, [this, groupFolder](FixtureGroup *, int row){
            beginInsertRows(indexForResource(groupFolder), row, row);
        });
        connect(collection, &FixtureGroupCollection::groupWasAddedAt, this, [this, groupFolder, read](FixtureGroup *, int){
            m_impl->refreshFlat(groupFolder, read);
            endInsertRows();
        });
        connect(collection, &FixtureGroupCollection::groupWillBeRemovedAt, this, [this, groupFolder](FixtureGroup *, int row){
            beginRemoveRows(indexForResource(groupFolder), row, row);
        });
        connect(collection, &FixtureGroupCollection::groupWasRemovedAt, this, [this, groupFolder, read](FixtureGroup *, int){
            m_impl->refreshFlat(groupFolder, read);
            endRemoveRows();
        });
    }

    // Routines.
    {
        auto *collection = t_project->routines();
        auto read = [collection](){
            QList<ProjectResource*> list;
            for(auto *routine : collection->routines())
                list.append(routine);
            return list;
        };
        m_impl->refreshFlat(routineFolder, read);

        connect(collection, &RoutineCollection::routineWillBeAdded, this, [this, routineFolder](Routine *, int row){
            beginInsertRows(indexForResource(routineFolder), row, row);
        });
        connect(collection, &RoutineCollection::routineWasAdded, this, [this, routineFolder, read](Routine *){
            m_impl->refreshFlat(routineFolder, read);
            endInsertRows();
        });
        connect(collection, &RoutineCollection::routineWillBeRemoved, this, [this, routineFolder](Routine *, int row){
            beginRemoveRows(indexForResource(routineFolder), row, row);
        });
        connect(collection, &RoutineCollection::routineWasRemoved, this, [this, routineFolder, read](Routine *){
            m_impl->refreshFlat(routineFolder, read);
            endRemoveRows();
        });
    }

    // Sequences - owned by PhotonCore, not the project, since they are separate
    // documents. Listed here all the same. Guarded because the collection hangs
    // off the application object, which need not exist (tests construct a bare
    // Project); everything else here comes from the project itself.
    if(auto *collection = photonApp ? photonApp->sequences() : nullptr)
    {
        auto read = [collection](){
            QList<ProjectResource*> list;
            for(auto *sequence : collection->sequences())
                list.append(sequence);
            return list;
        };
        m_impl->refreshFlat(sequenceFolder, read);

        connect(collection, &SequenceCollection::sequenceWillBeAdded, this, [this, sequenceFolder](Sequence *, int row){
            beginInsertRows(indexForResource(sequenceFolder), row, row);
        });
        connect(collection, &SequenceCollection::sequenceWasAdded, this, [this, sequenceFolder, read](Sequence *, int){
            m_impl->refreshFlat(sequenceFolder, read);
            endInsertRows();
        });
        connect(collection, &SequenceCollection::sequenceWillBeRemoved, this, [this, sequenceFolder](Sequence *, int row){
            beginRemoveRows(indexForResource(sequenceFolder), row, row);
        });
        connect(collection, &SequenceCollection::sequenceWasRemoved, this, [this, sequenceFolder, read](Sequence *, int){
            m_impl->refreshFlat(sequenceFolder, read);
            endRemoveRows();
        });
    }

    // Surfaces.
    {
        auto *collection = t_project->surfaces();
        auto read = [collection](){
            QList<ProjectResource*> list;
            for(auto *surface : collection->surfaces())
                list.append(surface);
            return list;
        };
        m_impl->refreshFlat(surfaceFolder, read);

        connect(collection, &SurfaceCollection::surfaceWillBeAdded, this, [this, surfaceFolder](Surface *, int row){
            beginInsertRows(indexForResource(surfaceFolder), row, row);
        });
        connect(collection, &SurfaceCollection::surfaceWasAdded, this, [this, surfaceFolder, read](Surface *, int){
            m_impl->refreshFlat(surfaceFolder, read);
            endInsertRows();
        });
        connect(collection, &SurfaceCollection::surfaceWillBeRemoved, this, [this, surfaceFolder](Surface *, int row){
            beginRemoveRows(indexForResource(surfaceFolder), row, row);
        });
        connect(collection, &SurfaceCollection::surfaceWasRemoved, this, [this, surfaceFolder, read](Surface *, int){
            m_impl->refreshFlat(surfaceFolder, read);
            endRemoveRows();
        });
    }

    // Pixel layouts.
    {
        auto *collection = t_project->pixelLayouts();
        auto read = [collection](){
            QList<ProjectResource*> list;
            for(auto *layout : collection->layouts())
                list.append(layout);
            return list;
        };
        m_impl->refreshFlat(layoutFolder, read);

        connect(collection, &PixelLayoutCollection::layoutWillBeAdded, this, [this, layoutFolder](PixelLayout *, int row){
            beginInsertRows(indexForResource(layoutFolder), row, row);
        });
        connect(collection, &PixelLayoutCollection::layoutWasAdded, this, [this, layoutFolder, read](PixelLayout *, int){
            m_impl->refreshFlat(layoutFolder, read);
            endInsertRows();
        });
        connect(collection, &PixelLayoutCollection::layoutWillBeRemoved, this, [this, layoutFolder](PixelLayout *, int row){
            beginRemoveRows(indexForResource(layoutFolder), row, row);
        });
        connect(collection, &PixelLayoutCollection::layoutWasRemoved, this, [this, layoutFolder, read](PixelLayout *, int){
            m_impl->refreshFlat(layoutFolder, read);
            endRemoveRows();
        });
    }

    // ---- Rig subtree -------------------------------------------------------
    // Only the descendant* signals bubble up the scene graph; the child*
    // structural ones are emitted by the immediate parent alone. So every
    // object in the tree has to be watched, not just the root, or edits below
    // the top level never reach the view.
    if(auto *root = t_project->sceneRoot())
    {
        m_impl->watchSceneSubtree(root);

        connect(root, &SceneObject::descendantModified, this, [this](SceneObject *object){
            m_impl->resourceUpdated(object);
        });
    }
}

ProjectModel::~ProjectModel()
{
    delete m_impl;
}

// ---- Scene relays ----------------------------------------------------------

void ProjectModel::Impl::watchSceneSubtree(SceneObject *t_object)
{
    if(!t_object)
        return;

    QObject::connect(t_object, &SceneObject::childWillBeAdded, facade,
                     [this](SceneObject *parent, SceneObject *child){ childWillBeAdded(parent, child); });
    QObject::connect(t_object, &SceneObject::childWasAdded, facade,
                     [this](SceneObject *child){ childWasAdded(child); });
    QObject::connect(t_object, &SceneObject::childWillBeRemoved, facade,
                     [this](SceneObject *parent, SceneObject *child){ childWillBeRemoved(parent, child); });
    QObject::connect(t_object, &SceneObject::childWasRemoved, facade,
                     [this](SceneObject *child){ childWasRemoved(child); });
    QObject::connect(t_object, &SceneObject::childWillBeMoved, facade,
                     [this](SceneObject *child, int newIndex){ childWillBeMoved(child, newIndex); });
    QObject::connect(t_object, &SceneObject::childWasMoved, facade,
                     [this](SceneObject *child){ childWasMoved(child); });

    for(auto *child : t_object->sceneChildren())
        watchSceneSubtree(child);
}

bool ProjectModel::Impl::isInScene(SceneObject *t_object) const
{
    SceneObject *root = sceneRoot();
    if(!t_object || !root)
        return false;
    if(t_object == root)
        return true;

    SceneObject *parent = t_object->parentSceneObject();
    if(!parent)
        return false;

    // Checked against the parent's child list rather than by following parent
    // pointers alone: setParentSceneObject(nullptr) drops the object from its
    // parent's sceneChildren() but deliberately leaves the QObject parent in
    // place (so ownership, and therefore destruction, still works). A detached
    // object therefore still reports a parent.
    if(!parent->sceneChildren().contains(t_object))
        return false;

    return isInScene(parent);
}

void ProjectModel::Impl::childWillBeAdded(SceneObject *t_parent, SceneObject *t_child)
{
    Q_UNUSED(t_child)

    pendingInsert = isInScene(t_parent);
    if(!pendingInsert)
        return;

    const QModelIndex parentIndex = facade->indexForResource(t_parent);
    const int row = t_parent->childCount();
    facade->beginInsertRows(parentIndex, row, row);
}

void ProjectModel::Impl::childWasAdded(SceneObject *t_child)
{
    if(!pendingInsert)
        return;
    pendingInsert = false;

    // A whole branch can arrive at once (a clone, or a deserialized subtree),
    // so watch everything under it, not just the object itself.
    watchSceneSubtree(t_child);
    watchResource(t_child);

    facade->endInsertRows();
}

void ProjectModel::Impl::childWillBeRemoved(SceneObject *t_parent, SceneObject *t_child)
{
    pendingRemove = isInScene(t_parent);
    if(!pendingRemove)
        return;

    const QModelIndex parentIndex = facade->indexForResource(t_parent);
    const int row = t_child->index();
    facade->beginRemoveRows(parentIndex, row, row);
}

void ProjectModel::Impl::childWasRemoved(SceneObject *)
{
    if(!pendingRemove)
        return;
    pendingRemove = false;
    facade->endRemoveRows();
}

void ProjectModel::Impl::childWillBeMoved(SceneObject *t_child, int t_newIndex)
{
    pendingMove = false;
    if(!isInScene(t_child))
        return;

    const QModelIndex parentIndex = facade->indexForResource(t_child->parentSceneObject());
    // beginMoveRows rejects a no-op move (destination inside the moved range);
    // honour its contract rather than letting it warn and return false.
    const int row = t_child->index();
    if(t_newIndex == row || t_newIndex == row + 1)
        return;

    pendingMove = facade->beginMoveRows(parentIndex, row, row, parentIndex, t_newIndex);
}

void ProjectModel::Impl::childWasMoved(SceneObject *)
{
    if(!pendingMove)
        return;
    pendingMove = false;
    facade->endMoveRows();
}

// ---- Index plumbing --------------------------------------------------------

ProjectResource *ProjectModel::resourceForIndex(const QModelIndex &t_index) const
{
    if(!t_index.isValid())
        return nullptr;
    return static_cast<ProjectResource*>(t_index.internalPointer());
}

ProjectFolder *ProjectModel::folderForContentType(const QByteArray &t_contentTypeId) const
{
    for(auto *folder : m_impl->folders)
        if(folder->contentTypeId() == t_contentTypeId)
            return folder;
    return nullptr;
}

QModelIndex ProjectModel::indexForResource(ProjectResource *t_resource) const
{
    if(!t_resource)
        return QModelIndex();

    // Folders are the top-level rows.
    if(auto *folder = dynamic_cast<ProjectFolder*>(t_resource))
    {
        const int row = m_impl->folders.indexOf(folder);
        return row < 0 ? QModelIndex() : createIndex(row, 0, static_cast<ProjectResource*>(folder));
    }

    if(auto *sceneObject = dynamic_cast<SceneObject*>(t_resource))
    {
        // The scene root isn't a row of its own - it *is* the Rig folder.
        if(sceneObject == m_impl->sceneRoot())
            return indexForResource(m_impl->rigFolder);

        return createIndex(sceneObject->index(), 0, static_cast<ProjectResource*>(sceneObject));
    }

    const int row = m_impl->rowOfResource(t_resource);
    return row < 0 ? QModelIndex() : createIndex(row, 0, t_resource);
}

QModelIndex ProjectModel::index(int t_row, int t_column, const QModelIndex &t_parent) const
{
    if(!hasIndex(t_row, t_column, t_parent))
        return QModelIndex();

    // Top level: the folders.
    if(!t_parent.isValid())
        return createIndex(t_row, t_column, static_cast<ProjectResource*>(m_impl->folders.at(t_row)));

    ProjectResource *parentResource = resourceForIndex(t_parent);

    if(auto *folder = dynamic_cast<ProjectFolder*>(parentResource))
    {
        const QList<ProjectResource*> children = m_impl->childrenOf(folder);
        if(t_row >= children.size())
            return QModelIndex();
        return createIndex(t_row, t_column, children.at(t_row));
    }

    // Inside the Rig subtree: walk the live scene graph.
    if(auto *sceneObject = dynamic_cast<SceneObject*>(parentResource))
    {
        if(SceneObject *child = sceneObject->childAtIndex(t_row))
            return createIndex(t_row, t_column, static_cast<ProjectResource*>(child));
    }

    return QModelIndex();
}

QModelIndex ProjectModel::parent(const QModelIndex &t_index) const
{
    ProjectResource *resource = resourceForIndex(t_index);
    if(!resource)
        return QModelIndex();

    // Folders sit at the root.
    if(dynamic_cast<ProjectFolder*>(resource))
        return QModelIndex();

    if(auto *sceneObject = dynamic_cast<SceneObject*>(resource))
    {
        SceneObject *parentObject = sceneObject->parentSceneObject();
        if(!parentObject)
            return QModelIndex();

        // Children of the scene root hang off the Rig folder row.
        if(parentObject == m_impl->sceneRoot())
            return indexForResource(m_impl->rigFolder);

        return indexForResource(parentObject);
    }

    return indexForResource(m_impl->folderForResource(resource));
}

int ProjectModel::rowCount(const QModelIndex &t_parent) const
{
    if(t_parent.column() > 0)
        return 0;

    if(!t_parent.isValid())
        return m_impl->folders.size();

    ProjectResource *resource = resourceForIndex(t_parent);

    if(auto *folder = dynamic_cast<ProjectFolder*>(resource))
    {
        if(folder == m_impl->rigFolder)
            return m_impl->sceneRoot() ? m_impl->sceneRoot()->childCount() : 0;
        return m_impl->flatChildren.value(folder).size();
    }

    if(auto *sceneObject = dynamic_cast<SceneObject*>(resource))
        return sceneObject->childCount();

    // Flat resources have no children.
    return 0;
}

int ProjectModel::columnCount(const QModelIndex &) const
{
    return ColumnCount;
}

// ---- Data ------------------------------------------------------------------

QVariant ProjectModel::data(const QModelIndex &t_index, int t_role) const
{
    ProjectResource *resource = resourceForIndex(t_index);
    if(!resource)
        return QVariant();

    auto *folder = dynamic_cast<ProjectFolder*>(resource);
    auto *sceneObject = dynamic_cast<SceneObject*>(resource);

    switch(t_role)
    {
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch(t_index.column())
            {
                case NameColumn:
                    return resource->resourceName();
                case TagsColumn:
                    return folder ? QString() : resource->resourceTags().join(", ");
                default:
                    return QVariant();
            }

        case Qt::ToolTipRole:
            return resource->resourceName();

        case Qt::CheckStateRole:
            // Visibility is a scene concept only.
            if(t_index.column() == VisibleColumn && sceneObject)
                return sceneObject->isVisible() ? Qt::Checked : Qt::Unchecked;
            return QVariant();

        case Qt::SizeHintRole:
            return QSize{32, 32};

        case TagsRole:
            return resource->resourceTags().join(' ');

        case TypeRole:
            return QString::fromLatin1(resource->resourceTypeId());

        case IsFolderRole:
            return folder != nullptr;
    }

    return QVariant();
}

bool ProjectModel::setData(const QModelIndex &t_index, const QVariant &t_value, int t_role)
{
    ProjectResource *resource = resourceForIndex(t_index);
    if(!resource || dynamic_cast<ProjectFolder*>(resource))
        return false;

    if(t_role == Qt::EditRole && t_index.column() == NameColumn)
    {
        resource->setResourceName(t_value.toString());
        return true;
    }

    if(t_role == Qt::CheckStateRole && t_index.column() == VisibleColumn)
    {
        if(auto *sceneObject = dynamic_cast<SceneObject*>(resource))
        {
            sceneObject->setVisible(t_value.toInt() == Qt::Checked);
            return true;
        }
    }

    return false;
}

QVariant ProjectModel::headerData(int t_section, Qt::Orientation t_orientation, int t_role) const
{
    if(t_orientation != Qt::Horizontal)
        return QVariant();

    if(t_role == Qt::DisplayRole)
    {
        switch(t_section)
        {
            case NameColumn:    return "Name";
            case VisibleColumn: return "Visible";
            case TagsColumn:    return "Tags";
            default:            return QVariant();
        }
    }

    if(t_role == Qt::ToolTipRole && t_section == VisibleColumn)
        return "Show/hide in the visualizer";

    return QVariant();
}

Qt::ItemFlags ProjectModel::flags(const QModelIndex &t_index) const
{
    if(!t_index.isValid())
        return Qt::NoItemFlags;

    ProjectResource *resource = resourceForIndex(t_index);
    auto *sceneObject = dynamic_cast<SceneObject*>(resource);

    // Folders are structural: selectable so the Add button can target them, but
    // never renameable, draggable, or checkable. The Rig folder does accept
    // drops - that's how you move an object back to the top of the hierarchy.
    if(auto *folder = dynamic_cast<ProjectFolder*>(resource))
    {
        Qt::ItemFlags folderFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        if(folder == m_impl->rigFolder)
            folderFlags |= Qt::ItemIsDropEnabled;
        return folderFlags;
    }

    Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if(t_index.column() == NameColumn)
        itemFlags |= Qt::ItemIsEditable;

    if(sceneObject)
    {
        // Only the Rig subtree takes part in drag/drop - the flat folders have
        // no hierarchy to rearrange.
        itemFlags |= Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
        if(t_index.column() == VisibleColumn)
            itemFlags |= Qt::ItemIsUserCheckable;
    }

    return itemFlags;
}

// ---- Drag and drop ---------------------------------------------------------

Qt::DropActions ProjectModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction;
}

Qt::DropActions ProjectModel::supportedDragActions() const
{
    return Qt::CopyAction;
}

QStringList ProjectModel::mimeTypes() const
{
    return QStringList() << SceneObject::SceneObjectMime << TagMimeType;
}

QMimeData *ProjectModel::mimeData(const QModelIndexList &t_indexes) const
{
    QVector<SceneObject*> objects;

    for(const QModelIndex &index : t_indexes)
    {
        if(auto *sceneObject = dynamic_cast<SceneObject*>(resourceForIndex(index)))
            if(!objects.contains(sceneObject))
                objects.append(sceneObject);
    }

    if(objects.isEmpty())
        return nullptr;

    return encodeSceneObjectMime(objects);
}

bool ProjectModel::canDropMimeData(const QMimeData *t_data, Qt::DropAction, int, int, const QModelIndex &t_parent) const
{
    if(!t_data)
        return false;

    if(t_data->hasFormat(TagMimeType))
    {
        // A tag can land on any real resource - not restricted to the Rig
        // subtree the way scene reparenting is, and not on a folder (there's
        // nothing there to tag).
        ProjectResource *target = resourceForIndex(t_parent);
        return target && !dynamic_cast<ProjectFolder*>(target);
    }

    if(!t_data->hasFormat(SceneObject::SceneObjectMime))
        return false;

    ProjectResource *parentResource = resourceForIndex(t_parent);

    // Dropping onto the Rig folder itself reparents to the scene root.
    if(auto *folder = dynamic_cast<ProjectFolder*>(parentResource))
        return folder == m_impl->rigFolder;

    // Anywhere else in the tree: only within the Rig subtree. Scene objects
    // have no meaning under Routines or Sequences.
    if(!dynamic_cast<SceneObject*>(parentResource))
        return false;

    // Don't let an object be dropped into its own subtree - that would detach
    // the branch from the tree entirely.
    const QVector<SceneObject*> dragged = decodeSceneObjectMime(t_data);
    auto *target = dynamic_cast<SceneObject*>(parentResource);
    for(auto *object : dragged)
    {
        for(SceneObject *ancestor = target; ancestor; ancestor = ancestor->parentSceneObject())
            if(ancestor == object)
                return false;
    }

    return true;
}

bool ProjectModel::dropMimeData(const QMimeData *t_data, Qt::DropAction t_action, int t_row, int t_column, const QModelIndex &t_parent)
{
    if(!canDropMimeData(t_data, t_action, t_row, t_column, t_parent))
        return false;

    if(t_data->hasFormat(TagMimeType))
    {
        ProjectResource *target = resourceForIndex(t_parent);
        if(!target)
            return false;

        for(const QString &tag : decodeTagMime(t_data))
            target->addResourceTag(tag);

        return true;
    }

    const QVector<SceneObject*> objects = decodeSceneObjectMime(t_data);
    if(objects.isEmpty())
        return false;

    ProjectResource *parentResource = resourceForIndex(t_parent);

    SceneObject *newParent = dynamic_cast<SceneObject*>(parentResource);
    if(!newParent)
        newParent = m_impl->sceneRoot();   // dropped on the Rig folder
    if(!newParent)
        return false;

    int row = t_row;
    if(row == -1)
        row = t_parent.isValid() ? 0 : newParent->childCount();

    QModelIndexList movedIndices;
    for(auto *object : objects)
    {
        if(object->parentSceneObject() == newParent)
        {
            newParent->moveChildToIndex(object, row);
        }
        else
        {
            object->setParentSceneObject(newParent, row);
            ++row;
        }
        movedIndices.append(indexForResource(object));
    }

    emit selectionMoved(movedIndices);
    return true;
}

} // namespace photon
