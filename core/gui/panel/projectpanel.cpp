#include <QApplication>
#include <QDrag>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QTimer>
#include "projectpanel_p.h"
#include "gui/dialog/fixturelibrarydialog.h"
#include "gui/tag/tagchip.h"
#include "gui/tag/tagmime.h"
#include "photoncore.h"
#include "project/project.h"
#include "fixture/fixture.h"
#include "fixture/fixturecollection.h"
#include "scene/scenedirection.h"
#include "scene/sceneaxis.h"
#include "scene/sceneboundaryrectangle.h"
#include "scene/sceneboundaryoval.h"
#include "scene/scenepointmarker.h"
#include "scene/scenelinearfalloff.h"
#include "fixture/fixturegroup.h"
#include "pixel/pixellayout.h"
#include "pixel/pixellayoutcollection.h"
#include "pixel/pixelstrip.h"
#include "routine/routine.h"
#include "routine/routinecollection.h"
#include "scene/scenearrow.h"
#include "scene/scenegroup.h"
#include "scene/sceneiterator.h"
#include "scene/scenesurface.h"
#include "scene/scenezone.h"
#include "scene/truss.h"
#include "sequence/sequence.h"
#include "sequence/sequencecollection.h"
#include "surface/surface.h"
#include "surface/surfacecollection.h"
namespace photon {

namespace {

// Every top-level category, in display order, paired with the resourceTypeId
// (or, for the Rig, the folder's contentTypeId) it corresponds to. Shared by
// the "Types" filter menu and the full Add menu so the two stay in sync.
const QVector<QPair<QString, QByteArray>> &projectCategories()
{
    static const QVector<QPair<QString, QByteArray>> categories{
        {"Rig",            "scene"},
        {"Fixture Groups", "group"},
        {"Routines",       "routine"},
        {"Sequences",      "sequence"},
        {"Surfaces",       "surface"},
        {"Pixel Layouts",  "pixel-layout"},
    };
    return categories;
}

// The category a clicked row belongs to - a folder's own contentTypeId, a
// scene object's "scene", or a flat resource's own resourceTypeId. Empty for
// no resource (e.g. a click on empty space), which callers treat as "show
// everything" rather than "show nothing".
QByteArray contentTypeForResource(ProjectResource *t_resource)
{
    if(!t_resource)
        return QByteArray();
    if(auto *folder = dynamic_cast<ProjectFolder*>(t_resource))
        return folder->contentTypeId();
    if(dynamic_cast<SceneObject*>(t_resource))
        return QByteArray("scene");
    return t_resource->resourceTypeId();
}

} // namespace

// ---- Filter proxy ----------------------------------------------------------

ProjectFilterModel::ProjectFilterModel(QObject *parent)
    : QSortFilterProxyModel{parent}
{
    setRecursiveFilteringEnabled(true);
}

void ProjectFilterModel::setSearchText(const QString &t_text)
{
    m_search = t_text.trimmed();
    invalidateFilter();
}

void ProjectFilterModel::setTypeFilter(const QSet<QByteArray> &t_types)
{
    m_types = t_types;
    invalidateFilter();
}

bool ProjectFilterModel::filterAcceptsRow(int t_sourceRow, const QModelIndex &t_sourceParent) const
{
    const QModelIndex index = sourceModel()->index(t_sourceRow, ProjectModel::NameColumn, t_sourceParent);
    if(!index.isValid())
        return false;

    // Folders are kept by recursive filtering when a descendant matches; on
    // their own they always pass so an empty folder is still visible as a place
    // to add things.
    if(index.data(ProjectModel::IsFolderRole).toBool())
    {
        if(m_types.isEmpty())
            return true;
        // With a type filter on, only show folders that could hold a match.
        // The Rig folder holds many scene types, so it is judged by its
        // children via recursive filtering instead.
        return true;
    }

    if(!m_types.isEmpty() && !m_types.contains(index.data(ProjectModel::TypeRole).toString().toLatin1()))
        return false;

    if(m_search.isEmpty())
        return true;

    if(index.data(Qt::DisplayRole).toString().contains(m_search, Qt::CaseInsensitive))
        return true;

    return index.data(ProjectModel::TagsRole).toString().contains(m_search, Qt::CaseInsensitive);
}

// ---- Tag delegate -----------------------------------------------------------

ProjectTagDelegate::ProjectTagDelegate(std::function<ProjectResource*(const QModelIndex&)> t_resolver, QObject *parent)
    : QStyledItemDelegate{parent}, m_resolver(t_resolver)
{
}

// Lays tags out left-to-right within rect, stopping once the next one
// wouldn't fit (always placing at least one, even if it has to be clipped,
// rather than showing nothing). paint() draws a "+N" marker for whatever's
// left over; chipAt() only ever needs to test against what's returned here.
QVector<QPair<QString, QRect>> ProjectTagDelegate::layoutChips(const QRect &t_rect, const QStringList &t_tags, const QFontMetrics &t_metrics) const
{
    QVector<QPair<QString, QRect>> result;
    int x = t_rect.left();
    constexpr int spacing = 4;

    for(const QString &tag : t_tags)
    {
        const QSize size = tagChipSizeHint(t_metrics, tag);
        if(x + size.width() > t_rect.right() && !result.isEmpty())
            break;

        result.append({tag, QRect(x, t_rect.top() + (t_rect.height() - size.height()) / 2, size.width(), size.height())});
        x += size.width() + spacing;
    }

    return result;
}

void ProjectTagDelegate::paint(QPainter *t_painter, const QStyleOptionViewItem &t_option, const QModelIndex &t_index) const
{
    QStyleOptionViewItem option = t_option;
    initStyleOption(&option, t_index);

    // Selection/hover background, same as the default delegate, but with no
    // text of its own - the chips take its place.
    option.text.clear();
    QStyle *style = option.widget ? option.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &option, t_painter, option.widget);

    ProjectResource *resource = m_resolver ? m_resolver(t_index) : nullptr;
    if(!resource)
        return;   // folders carry no tags

    const QStringList tags = resource->resourceTags();
    if(tags.isEmpty())
        return;

    t_painter->save();
    t_painter->setClipRect(option.rect);

    const QRect contentRect = option.rect.adjusted(2, 0, -2, 0);
    const auto chips = layoutChips(contentRect, tags, option.fontMetrics);

    for(const auto &chip : chips)
        paintTagChip(t_painter, chip.second, chip.first);

    if(chips.size() < tags.size())
    {
        const int remaining = tags.size() - chips.size();
        const int left = chips.isEmpty() ? contentRect.left() : chips.last().second.right() + 4;
        const QRect overflowRect(left, contentRect.top(), contentRect.right() - left, contentRect.height());
        t_painter->setPen(option.palette.color(QPalette::Disabled, QPalette::Text));
        t_painter->drawText(overflowRect, Qt::AlignVCenter | Qt::AlignLeft, QString("+%1").arg(remaining));
    }

    t_painter->restore();
}

QString ProjectTagDelegate::chipAt(QTreeView *t_view, const QModelIndex &t_index, const QPoint &t_pos) const
{
    ProjectResource *resource = m_resolver ? m_resolver(t_index) : nullptr;
    if(!resource)
        return {};

    const QStringList tags = resource->resourceTags();
    if(tags.isEmpty())
        return {};

    const QRect cellRect = t_view->visualRect(t_index);
    if(!cellRect.contains(t_pos))
        return {};

    const QRect contentRect = cellRect.adjusted(2, 0, -2, 0);
    for(const auto &chip : layoutChips(contentRect, tags, t_view->fontMetrics()))
        if(chip.second.contains(t_pos))
            return chip.first;

    return {};
}

// ---- Impl helpers ----------------------------------------------------------

ProjectResource *ProjectPanel::Impl::resourceAt(const QModelIndex &t_proxyIndex) const
{
    if(!t_proxyIndex.isValid() || !model || !proxy)
        return nullptr;
    return model->resourceForIndex(proxy->mapToSource(t_proxyIndex));
}

QList<ProjectResource*> ProjectPanel::Impl::selectedResources() const
{
    QList<ProjectResource*> resources;
    if(!treeView->selectionModel())
        return resources;

    for(const QModelIndex &index : treeView->selectionModel()->selectedRows())
    {
        if(auto *resource = resourceAt(index))
            resources.append(resource);
    }
    return resources;
}

// ---- Panel -----------------------------------------------------------------

ProjectPanel::ProjectPanel() : Panel("photon.project"), m_impl(new Impl)
{
    setName("Project");

    m_impl->layout = new QVBoxLayout;

    auto *filterRow = new QHBoxLayout;
    m_impl->searchEdit = new QLineEdit;
    m_impl->searchEdit->setPlaceholderText("Search name or tag");
    m_impl->searchEdit->setClearButtonEnabled(true);

    m_impl->typeButton = new QPushButton("Types");
    m_impl->typeButton->setToolTip("Choose which kinds of resource to show");

    filterRow->addWidget(m_impl->searchEdit, 1);
    filterRow->addWidget(m_impl->typeButton);
    m_impl->layout->addLayout(filterRow);

    m_impl->treeView = new QTreeView;
    m_impl->treeView->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    m_impl->treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_impl->treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_impl->treeView->setDragEnabled(true);
    m_impl->treeView->setDragDropMode(QAbstractItemView::DragDrop);
    m_impl->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_impl->layout->addWidget(m_impl->treeView);

    m_impl->tagDelegate = new ProjectTagDelegate([this](const QModelIndex &index){ return m_impl->resourceAt(index); }, this);
    m_impl->treeView->setItemDelegateForColumn(ProjectModel::TagsColumn, m_impl->tagDelegate);
    // Chips are painted, not child widgets, so drag-start detection for them
    // happens here rather than through the delegate's own event handling.
    m_impl->treeView->viewport()->installEventFilter(this);

    m_impl->addButton = new QPushButton("Add");
    m_impl->removeButton = new QPushButton("Remove");
    m_impl->duplicateButton = new QPushButton("Duplicate");
    m_impl->addButton->setEnabled(false);
    m_impl->removeButton->setEnabled(false);
    m_impl->duplicateButton->setEnabled(false);

    auto *buttonRow = new QHBoxLayout;
    buttonRow->addWidget(m_impl->addButton);
    buttonRow->addWidget(m_impl->removeButton);
    buttonRow->addWidget(m_impl->duplicateButton);
    m_impl->layout->addLayout(buttonRow);

    setPanelLayout(m_impl->layout);

    connect(m_impl->searchEdit, &QLineEdit::textChanged, this, &ProjectPanel::filterChanged);
    connect(m_impl->typeButton, &QPushButton::clicked, this, [this](){
        // Built fresh each time so it reflects the current check states.
        QMenu menu;

        for(const auto &type : projectCategories())
        {
            QAction *action = menu.addAction(type.first);
            action->setCheckable(true);
            action->setChecked(!m_impl->hiddenTypes.contains(type.second));
            connect(action, &QAction::toggled, this, [this, key = type.second](bool checked){
                if(checked)
                    m_impl->hiddenTypes.remove(key);
                else
                    m_impl->hiddenTypes.insert(key);
                filterChanged();
            });
        }

        menu.exec(m_impl->typeButton->mapToGlobal(m_impl->typeButton->rect().bottomLeft()));
    });

    connect(m_impl->addButton, &QPushButton::clicked, this, &ProjectPanel::addClicked);
    connect(m_impl->removeButton, &QPushButton::clicked, this, &ProjectPanel::removeClicked);
    connect(m_impl->duplicateButton, &QPushButton::clicked, this, &ProjectPanel::duplicateClicked);
    connect(m_impl->treeView, &QTreeView::doubleClicked, this, &ProjectPanel::doubleClicked);
    connect(m_impl->treeView, &QWidget::customContextMenuRequested, this, &ProjectPanel::contextMenuRequested);
}

ProjectPanel::~ProjectPanel()
{
    delete m_impl;
}

bool ProjectPanel::eventFilter(QObject *t_watched, QEvent *t_event)
{
    if(t_watched != m_impl->treeView->viewport())
        return Panel::eventFilter(t_watched, t_event);

    if(t_event->type() == QEvent::MouseButtonPress)
    {
        auto *mouseEvent = static_cast<QMouseEvent*>(t_event);
        if(mouseEvent->button() == Qt::LeftButton)
        {
            // Set before the event propagates to the tree's own
            // mousePressEvent, so if that immediately changes the selection
            // (the common case: pressing an unselected row), selectionChanged
            // sees resolvingClickOrDrag already true and defers publishing it.
            m_impl->resolvingClickOrDrag = true;
            m_impl->pendingSelectionPublish = false;

            const QModelIndex index = m_impl->treeView->indexAt(mouseEvent->pos());
            if(index.isValid() && index.column() == ProjectModel::TagsColumn)
                m_impl->pendingDragTag = m_impl->tagDelegate->chipAt(m_impl->treeView, index, mouseEvent->pos());
            else
                m_impl->pendingDragTag.clear();
            m_impl->pendingDragPos = mouseEvent->pos();
        }
    }
    else if(t_event->type() == QEvent::MouseMove)
    {
        auto *mouseEvent = static_cast<QMouseEvent*>(t_event);
        const bool pastDragDistance = (mouseEvent->buttons() & Qt::LeftButton)
            && (mouseEvent->pos() - m_impl->pendingDragPos).manhattanLength() >= QApplication::startDragDistance();

        if(!m_impl->pendingDragTag.isEmpty() && pastDragDistance)
        {
            const QString tag = m_impl->pendingDragTag;
            m_impl->pendingDragTag.clear();
            // This gesture just resolved as a drag, not a click - whatever
            // selection change the initiating press caused is discarded
            // rather than published, same as the native-row-drag case below.
            m_impl->resolvingClickOrDrag = false;
            m_impl->pendingSelectionPublish = false;

            auto *drag = new QDrag(m_impl->treeView);
            drag->setMimeData(encodeTagMime({tag}));

            QPixmap pixmap(tagChipSizeHint(m_impl->treeView->fontMetrics(), tag));
            pixmap.fill(Qt::transparent);
            QPainter pixmapPainter(&pixmap);
            paintTagChip(&pixmapPainter, pixmap.rect(), tag);
            pixmapPainter.end();
            drag->setPixmap(pixmap);

            // The scene-object row-drag path (ProjectModel::mimeData()) isn't
            // involved here at all - this is a manually started drag carrying
            // only the one chip that was pressed.
            drag->exec(Qt::CopyAction);
            return true;
        }

        // Not a chip drag, but still resolving and past the threshold: this
        // is about to become the tree's own native row drag (reparenting),
        // handled entirely by QTreeView's base mouseMoveEvent once this
        // returns - discard any pending publish from the initiating press
        // the same way, so dragging an unselected row away doesn't flash the
        // Properties panel to it either. A drop that legitimately changes the
        // selection (ProjectModel::selectionMoved) publishes on its own,
        // since resolvingClickOrDrag is false by the time that happens.
        if(m_impl->resolvingClickOrDrag && pastDragDistance)
        {
            m_impl->resolvingClickOrDrag = false;
            m_impl->pendingSelectionPublish = false;
        }
    }
    else if(t_event->type() == QEvent::MouseButtonRelease)
    {
        m_impl->pendingDragTag.clear();

        // Set before the event reaches the tree's own mouseReleaseEvent: if
        // releasing over an already-multi-selected row collapses the
        // selection to just that row, selectionChanged fires synchronously
        // from within that call and, seeing resolvingClickOrDrag already
        // false, publishes it immediately - no separate handling needed here
        // for that case.
        m_impl->resolvingClickOrDrag = false;

        if(m_impl->pendingSelectionPublish)
        {
            m_impl->pendingSelectionPublish = false;
            // Deferred a tick so this runs after the tree's own release
            // handling (and any selectionChanged it triggers) has settled,
            // rather than racing it.
            QTimer::singleShot(0, this, [this](){
                if(photonApp->project())
                    photonApp->project()->setSelectedResources(m_impl->selectedResources());
            });
        }
    }

    return Panel::eventFilter(t_watched, t_event);
}

void ProjectPanel::projectDidOpen(Project *t_project)
{
    m_impl->model = new ProjectModel(t_project, this);
    m_impl->proxy = new ProjectFilterModel(this);
    m_impl->proxy->setSourceModel(m_impl->model);

    m_impl->treeView->setModel(m_impl->proxy);
    m_impl->treeView->header()->setSectionResizeMode(ProjectModel::NameColumn, QHeaderView::Stretch);
    m_impl->treeView->header()->setSectionResizeMode(ProjectModel::VisibleColumn, QHeaderView::ResizeToContents);
    m_impl->treeView->expandToDepth(0);

    connect(m_impl->treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ProjectPanel::selectionChanged);
    connect(m_impl->model, &ProjectModel::selectionMoved, this, &ProjectPanel::selectionMoved);
    connect(t_project, &Project::selectedResourcesChanged, this, &ProjectPanel::syncSelectionFromProject);

    m_impl->addButton->setEnabled(true);
}

void ProjectPanel::projectWillClose(Project *t_project)
{
    disconnect(t_project, &Project::selectedResourcesChanged, this, &ProjectPanel::syncSelectionFromProject);
    if(m_impl->treeView->selectionModel())
        disconnect(m_impl->treeView->selectionModel(), &QItemSelectionModel::selectionChanged,
                   this, &ProjectPanel::selectionChanged);

    m_impl->treeView->setModel(nullptr);

    delete m_impl->proxy;
    m_impl->proxy = nullptr;
    delete m_impl->model;
    m_impl->model = nullptr;

    m_impl->addButton->setEnabled(false);
    m_impl->removeButton->setEnabled(false);
    m_impl->duplicateButton->setEnabled(false);
}

void ProjectPanel::filterChanged()
{
    if(!m_impl->proxy)
        return;

    QSet<QByteArray> visible;
    // The proxy takes an allow-list, but the menu is phrased as "show these",
    // so translate only when something is actually hidden.
    if(!m_impl->hiddenTypes.isEmpty())
    {
        for(const auto &category : projectCategories())
        {
            const QByteArray &type = category.second;
            if(m_impl->hiddenTypes.contains(type))
                continue;
            if(type == "scene")
            {
                // Scene objects report their concrete typeId, not "scene".
                visible.insert("fixture");
                visible.insert("group");
                visible.insert("truss");
                visible.insert("surface");
                visible.insert("zone");
                visible.insert("pixelstrip");
                visible.insert("arrow");
                visible.insert("direction");
                visible.insert("axis");
                visible.insert("boundaryrectangle");
                visible.insert("boundaryoval");
                visible.insert("pointmarker");
                visible.insert("linearfalloff");
            }
            else
                visible.insert(type);
        }
    }

    m_impl->proxy->setTypeFilter(visible);
    m_impl->proxy->setSearchText(m_impl->searchEdit->text());

    if(!m_impl->searchEdit->text().trimmed().isEmpty())
        m_impl->treeView->expandAll();
}

// ---- Selection -------------------------------------------------------------

void ProjectPanel::selectionChanged(const QItemSelection &, const QItemSelection &)
{
    const QList<ProjectResource*> selection = m_impl->selectedResources();

    const bool hasSelection = !selection.isEmpty();
    m_impl->removeButton->setEnabled(hasSelection);
    // Duplicate is a scene-graph operation for now.
    bool hasSceneObject = false;
    for(auto *resource : selection)
        if(dynamic_cast<SceneObject*>(resource))
            hasSceneObject = true;
    m_impl->duplicateButton->setEnabled(hasSceneObject);

    if(m_impl->syncingFromProject || !photonApp->project())
        return;

    // While still resolving whether this gesture is a click or a drag, this
    // selection change might be nothing more than the first half of a drag
    // (pressing an unselected row selects it before a drag can start) -
    // publishing it to Project now would flash the Properties panel to
    // whatever's under the cursor even when the user only meant to drag it
    // elsewhere without selecting it. Deferred to mouse release (a click) in
    // the event filter, and discarded entirely if it resolves as a drag
    // instead. Anything that isn't a live mouse press-drag-release on the
    // tree (keyboard navigation, programmatic selection, a completed drop)
    // publishes immediately, same as before.
    if(m_impl->resolvingClickOrDrag)
        m_impl->pendingSelectionPublish = true;
    else
        photonApp->project()->setSelectedResources(selection);
}

void ProjectPanel::syncSelectionFromProject(const QList<ProjectResource*> &t_resources)
{
    if(!m_impl->model || !m_impl->proxy)
        return;

    m_impl->syncingFromProject = true;

    if(t_resources.isEmpty())
    {
        m_impl->treeView->selectionModel()->clearSelection();
    }
    else
    {
        QItemSelection selection;
        QModelIndex last;
        for(auto *resource : t_resources)
        {
            const QModelIndex index = m_impl->proxy->mapFromSource(m_impl->model->indexForResource(resource));
            if(index.isValid())
            {
                selection.select(index, index);
                last = index;
            }
        }

        m_impl->treeView->selectionModel()->select(
            selection, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

        if(last.isValid())
            m_impl->treeView->scrollTo(last);
    }

    m_impl->syncingFromProject = false;
}

void ProjectPanel::selectionMoved(QModelIndexList t_indices)
{
    if(t_indices.isEmpty() || !m_impl->proxy)
        return;

    const QModelIndex index = m_impl->proxy->mapFromSource(t_indices.first());
    if(index.isValid())
        m_impl->treeView->selectionModel()->select(
            index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

// ---- Opening ---------------------------------------------------------------

void ProjectPanel::doubleClicked(const QModelIndex &t_index)
{
    ProjectResource *resource = m_impl->resourceAt(t_index);
    if(!resource)
        return;

    // Folders toggle rather than open.
    if(dynamic_cast<ProjectFolder*>(resource))
    {
        m_impl->treeView->setExpanded(t_index, !m_impl->treeView->isExpanded(t_index));
        return;
    }

    // Routines, Sequences and Surfaces are big enough to want their own
    // dedicated panel; a Fixture Group or Pixel Layout is compact enough to
    // live entirely inline in the Properties panel, so double-click has
    // nothing extra to do for those.
    if(auto *routine = dynamic_cast<Routine*>(resource))
        photonApp->editRoutine(routine);
    else if(auto *sequence = dynamic_cast<Sequence*>(resource))
        photonApp->sequences()->editSequence(sequence);
    else if(auto *surface = dynamic_cast<Surface*>(resource))
        photonApp->surfaces()->editSurface(surface);
}

// ---- Add / Remove / Duplicate ----------------------------------------------

void ProjectPanel::populateAddActions(QMenu &t_menu, const QByteArray &t_contentType, SceneObject *t_preferredParent)
{
    // Where new scene objects land: t_preferredParent if the caller pinned one
    // (the row that was right-clicked), else the selected scene object, else
    // the root. Evaluated when the action actually runs, not when the menu is
    // built, so it reflects the selection at that point.
    auto sceneParent = [this, t_preferredParent]() -> SceneObject* {
        if(t_preferredParent)
            return t_preferredParent;
        const QList<ProjectResource*> selection = m_impl->selectedResources();
        if(!selection.isEmpty())
            if(auto *sceneObject = dynamic_cast<SceneObject*>(selection.last()))
                return sceneObject;
        return photonApp->project()->sceneRoot();
    };

    if(t_contentType == "scene")
    {
        t_menu.addAction("Fixture", [this, sceneParent](){
            const QString loadPath = FixtureLibraryDialog::getFixturePath(this);
            if(loadPath.isEmpty())
                return;

            auto *fixture = new Fixture(loadPath);
            const auto sameName = photonApp->project()->fixtures()->fixturesWithName(fixture->name());
            if(sameName.length() > 0)
                fixture->setName(fixture->name() + " " + QString::number(sameName.length() + 1));

            fixture->setParentSceneObject(sceneParent());
        });
        t_menu.addAction("Group", [sceneParent](){
            auto *group = new SceneGroup;
            group->setName("Group");
            group->setParentSceneObject(sceneParent());
        });
        t_menu.addAction("Truss", [sceneParent](){
            auto *truss = new Truss;
            truss->setName("Truss");
            truss->setParentSceneObject(sceneParent());
        });
        t_menu.addAction("Light Strip", [sceneParent](){
            auto *strip = new PixelStrip;
            strip->setName("Pixel Strip");
            strip->setParentSceneObject(sceneParent());
        });
        t_menu.addAction("Arrow", [sceneParent](){
            auto *arrow = new SceneArrow;
            arrow->setName("Arrow");
            arrow->setParentSceneObject(sceneParent());
        });
        t_menu.addAction("Wall", [sceneParent](){
            auto *wall = new SceneSurface;
            wall->setName("Wall");
            wall->setSurfaceWidth(12.0f);
            wall->setSurfaceHeight(6.0f);
            // Vertical plane standing on the floor, set back behind the rig.
            wall->setPosition(QVector3D(0.0f, 3.0f, -6.0f));
            wall->setParentSceneObject(sceneParent());
        });
        t_menu.addAction("Floor", [sceneParent](){
            auto *floor = new SceneSurface;
            floor->setName("Floor");
            floor->setSurfaceWidth(16.0f);
            floor->setSurfaceHeight(16.0f);
            // Lay the plane flat at ground level.
            floor->setRotation(QVector3D(-90.0f, 0.0f, 0.0f));
            floor->setPosition(QVector3D(0.0f, 0.0f, 0.0f));
            floor->setParentSceneObject(sceneParent());
        });
        t_menu.addAction("Zone", [sceneParent](){
            auto *zone = new SceneZone;
            zone->setName("Zone");
            zone->setSize(QVector3D(4.0f, 4.0f, 4.0f));
            zone->setPosition(QVector3D(0.0f, 2.0f, 0.0f));
            zone->setParentSceneObject(sceneParent());
        });
        t_menu.addAction("Direction", [sceneParent](){
            auto *direction = new SceneDirection;
            direction->setName("Direction");
            direction->setParentSceneObject(sceneParent());
        });
        t_menu.addAction("Axis", [sceneParent](){
            auto *axis = new SceneAxis;
            axis->setName("Axis");
            axis->setParentSceneObject(sceneParent());
        });
        t_menu.addAction("Boundary Rectangle", [sceneParent](){
            auto *rect = new SceneBoundaryRectangle;
            rect->setName("Boundary Rectangle");
            // Lay flat at ground level, matching its main use (marking a floor area).
            rect->setRotation(QVector3D(-90.0f, 0.0f, 0.0f));
            rect->setParentSceneObject(sceneParent());
        });
        t_menu.addAction("Boundary Oval", [sceneParent](){
            auto *oval = new SceneBoundaryOval;
            oval->setName("Boundary Oval");
            oval->setRotation(QVector3D(-90.0f, 0.0f, 0.0f));
            oval->setParentSceneObject(sceneParent());
        });
        t_menu.addAction("Point in Space", [sceneParent](){
            auto *marker = new ScenePointMarker;
            marker->setName("Point in Space");
            marker->setRotation(QVector3D(-90.0f, 0.0f, 0.0f));
            marker->setParentSceneObject(sceneParent());
        });
        t_menu.addAction("Linear Falloff", [sceneParent](){
            auto *falloff = new SceneLinearFalloff;
            falloff->setName("Linear Falloff");
            falloff->setParentSceneObject(sceneParent());
        });
    }
    else if(t_contentType == "group")
    {
        t_menu.addAction("Fixture Group", [this](){
            bool ok = false;
            const QString name = QInputDialog::getText(this, "Fixture Group", "Name", QLineEdit::Normal, "Untitled", &ok);
            if(ok && !name.isEmpty())
                photonApp->project()->groups()->addGroup(name);
        });
    }
    else if(t_contentType == "routine")
    {
        t_menu.addAction("Add Routine", [this](){
            bool ok = false;
            const QString name = QInputDialog::getText(this, "Routine", "Name", QLineEdit::Normal, "Untitled", &ok);
            if(ok && !name.isEmpty())
                photonApp->project()->routines()->addRoutine(new Routine(name));
        });
    }
    else if(t_contentType == "sequence")
    {
        t_menu.addAction("Add Sequence", [](){
            photonApp->newSequence();
        });
    }
    else if(t_contentType == "surface")
    {
        t_menu.addAction("Add Surface", [this](){
            bool ok = false;
            const QString name = QInputDialog::getText(this, "Surface", "Name", QLineEdit::Normal, "Untitled", &ok);
            if(ok && !name.isEmpty())
                photonApp->surfaces()->addSurface(new Surface(name));
        });
    }
    else if(t_contentType == "pixel-layout")
    {
        t_menu.addAction("Add Pixel Layout", [](){
            // A pixel layout has no dimensions of its own (unlike a Canvas) -
            // just create it, same as the plain Rig items (Group, Truss, ...).
            auto *layout = new PixelLayout;
            layout->setName("Pixel Layout");
            photonApp->project()->pixelLayouts()->addLayout(layout);
        });
    }
}

void ProjectPanel::populateFullAddMenu(QMenu &t_menu, SceneObject *t_preferredParent)
{
    // Every category gets its own submenu, uniformly - even the ones with a
    // single action - so the structure doesn't change shape as categories
    // grow (the Rig one already has eight).
    for(const auto &category : projectCategories())
    {
        QMenu *submenu = t_menu.addMenu(category.first);
        populateAddActions(*submenu, category.second, t_preferredParent);
    }
}

void ProjectPanel::addClicked()
{
    QMenu menu;
    populateFullAddMenu(menu);
    menu.exec(m_impl->addButton->mapToGlobal(m_impl->addButton->rect().bottomLeft()));
}

void ProjectPanel::removeClicked()
{
    const QList<ProjectResource*> selection = m_impl->selectedResources();
    m_impl->treeView->clearSelection();

    Project *project = photonApp->project();
    if(!project)
        return;

    for(auto *resource : selection)
    {
        if(auto *sceneObject = dynamic_cast<SceneObject*>(resource))
            sceneObject->setParentSceneObject(nullptr);
        else if(auto *group = dynamic_cast<FixtureGroup*>(resource))
            project->groups()->removeGroup(group);
        else if(auto *routine = dynamic_cast<Routine*>(resource))
            project->routines()->removeRoutine(routine);
        else if(auto *sequence = dynamic_cast<Sequence*>(resource))
            photonApp->sequences()->removeSequence(sequence);
        else if(auto *surface = dynamic_cast<Surface*>(resource))
            project->surfaces()->removeSurface(surface);
        else if(auto *layout = dynamic_cast<PixelLayout*>(resource))
            project->pixelLayouts()->removeLayout(layout);
    }
}

void ProjectPanel::duplicateClicked()
{
    // Carried over from RigPanel: cloned fixtures are renumbered past the
    // highest DMX address currently in use so they don't collide.
    auto fixtures = SceneIterator::FindMany(photonApp->project()->sceneRoot(), [](SceneObject *object, bool *){
        return dynamic_cast<Fixture*>(object) != nullptr;
    });

    int nextDMX = 0;
    for(auto *object : fixtures)
    {
        const int end = static_cast<Fixture*>(object)->dmxOffset() + static_cast<Fixture*>(object)->dmxSize();
        if(end > nextDMX)
            nextDMX = end;
    }

    for(auto *resource : m_impl->selectedResources())
    {
        auto *sceneObject = dynamic_cast<SceneObject*>(resource);
        if(!sceneObject)
            continue;

        SceneObject *clone = sceneObject->clone();
        if(!clone)
            continue;

        clone->setParentSceneObject(sceneObject->parentSceneObject());

        auto clonedFixtures = SceneIterator::FindMany(clone, [](SceneObject *object, bool *){
            return dynamic_cast<Fixture*>(object) != nullptr;
        });
        for(auto *object : clonedFixtures)
        {
            auto *fixture = static_cast<Fixture*>(object);
            fixture->setDMXOffset(nextDMX);
            nextDMX += fixture->dmxSize();
        }
    }
}

void ProjectPanel::contextMenuRequested(const QPoint &t_pos)
{
    const QModelIndex index = m_impl->treeView->indexAt(t_pos);
    ProjectResource *resource = m_impl->resourceAt(index);

    // Right-clicking a chip itself is scoped to just that chip - a minimal
    // "Remove Tag" menu, not the resource's usual Add/Rename/Duplicate/Delete
    // menu below.
    if(index.isValid() && index.column() == ProjectModel::TagsColumn && resource)
    {
        const QString tag = m_impl->tagDelegate->chipAt(m_impl->treeView, index, t_pos);
        if(!tag.isEmpty())
        {
            QMenu chipMenu;
            chipMenu.addAction("Remove Tag", [resource, tag](){
                resource->removeResourceTag(tag);
            });
            chipMenu.exec(m_impl->treeView->viewport()->mapToGlobal(t_pos));
            return;
        }
    }

    QMenu menu;

    // Right-clicking a specific category (a folder, a scene object, or a flat
    // resource like a Routine) shows just that category's own "New" actions,
    // directly - not a generic "Add..." that reopens the full menu. Empty
    // space has no category to infer, so it falls back to the full menu.
    const QByteArray contentType = contentTypeForResource(resource);
    if(!contentType.isEmpty())
        populateAddActions(menu, contentType, dynamic_cast<SceneObject*>(resource));
    else
        populateFullAddMenu(menu);

    if(resource && !dynamic_cast<ProjectFolder*>(resource))
    {
        if(!menu.isEmpty())
            menu.addSeparator();
        menu.addAction("Rename", [this, index](){
            m_impl->treeView->edit(index.sibling(index.row(), ProjectModel::NameColumn));
        });
        if(dynamic_cast<SceneObject*>(resource))
            menu.addAction("Duplicate", this, &ProjectPanel::duplicateClicked);
        menu.addSeparator();
        menu.addAction("Delete", this, &ProjectPanel::removeClicked);
    }

    if(menu.isEmpty())
        return;

    menu.exec(m_impl->treeView->viewport()->mapToGlobal(t_pos));
}

} // namespace photon
