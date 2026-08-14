
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMenu>
#include <QPainter>
#include <QResizeEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include "pixellayouteditor_p.h"
#include "pixelarrangedialog.h"
#include "pixel/pixelsourcelayout.h"
#include "pixel/pixellayout.h"
#include "pixel/pixelsource.h"
#include "pixel/pixelarrange.h"
#include "scene/sceneobject.h"
#include "scene/sceneiterator.h"
#include "scene/scenemodel.h"
#include "photoncore.h"
#include "project/project.h"
#include "fixture/fixture.h"
#include "fixture/capability/colorcapability.h"
#include "fixture/capability/fixturecapability.h"
#include "pixel/fixturepixelsource.h"

namespace photon {

PixelLayoutScene::PixelLayoutScene(PixelLayout *t_layout) : QGraphicsScene(),m_layout(t_layout)
{
    for(auto source : m_layout->sourceLayouts())
    {
        sourceAdded(source);
    }

    connect(m_layout, &PixelLayout::sourceWasAdded, this, &PixelLayoutScene::sourceAdded);
    connect(m_layout, &PixelLayout::sourceWillBeRemoved, this, &PixelLayoutScene::sourceRemoved);
}

void PixelLayoutScene::sourceAdded(photon::PixelSourceLayout *t_source)
{
    QVector<PixelPointItem*> items;
    int count = t_source->pixelCount();
    for(int i = 0; i < count; ++i)
    {
        auto *item = new PixelPointItem(t_source, i);
        addItem(item);
        item->reposition();
        item->setInteractive(t_source == m_activeSourceLayout);
        items << item;
    }
    m_pointItems.insert(t_source, items);

    // Reposition this source's points whenever a pixel position is written
    // programmatically (a drag repositions its own item directly via
    // itemChange(), but an Arrange command writes the model without ever
    // touching the item, so it needs this to actually redraw).
    connect(t_source, &PixelSourceLayout::pixelPositionsChanged, this, [this, t_source](){ repositionSource(t_source); });

    // Live pixel-count changes (e.g. PixelStrip::setPixelCount()) only reach
    // us via the underlying SceneObject's generic metadataChanged signal -
    // FixturePixelSource isn't a SceneObject and its count is fixed at
    // construction, so it never needs this. t_source is used as the
    // connection context (not just captured) so this auto-disconnects if
    // t_source is destroyed before the scene object is.
    if(auto *sceneObj = dynamic_cast<SceneObject*>(t_source->source()))
        connect(sceneObj, &SceneObject::metadataChanged, t_source, [this, t_source](){ syncPointCount(t_source); });
}

void PixelLayoutScene::sourceRemoved(photon::PixelSourceLayout *t_source)
{
    auto it = m_pointItems.find(t_source);
    if(it == m_pointItems.end())
        return;

    for(auto *item : it.value())
        delete item;
    m_pointItems.erase(it);

    if(m_activeSourceLayout == t_source)
        m_activeSourceLayout = nullptr;
}

void PixelLayoutScene::syncPointCount(PixelSourceLayout *t_source)
{
    auto it = m_pointItems.find(t_source);
    if(it == m_pointItems.end())
        return;

    int count = t_source->pixelCount();
    auto &items = it.value();

    while(items.size() > count)
        delete items.takeLast();

    while(items.size() < count)
    {
        auto *item = new PixelPointItem(t_source, items.size());
        addItem(item);
        item->reposition();
        item->setInteractive(t_source == m_activeSourceLayout);
        items << item;
    }
}

void PixelLayoutScene::repositionSource(PixelSourceLayout *t_source)
{
    auto it = m_pointItems.find(t_source);
    if(it == m_pointItems.end())
        return;

    for(auto *item : it.value())
        item->reposition();
}

void PixelLayoutScene::selectSource(PixelSourceLayout *t_source)
{
    if(m_activeSourceLayout == t_source)
        return;

    auto oldIt = m_pointItems.find(m_activeSourceLayout);
    if(oldIt != m_pointItems.end())
        for(auto *item : oldIt.value())
            item->setInteractive(false);

    m_activeSourceLayout = t_source;

    auto newIt = m_pointItems.find(m_activeSourceLayout);
    if(newIt != m_pointItems.end())
        for(auto *item : newIt.value())
            item->setInteractive(true);

    update();
}

QVector<QPair<PixelSourceLayout*,int>> PixelLayoutScene::selectedPixels() const
{
    QVector<QPair<PixelSourceLayout*,int>> result;
    for(auto *sourceLayout : m_layout->sourceLayouts())
    {
        auto it = m_pointItems.find(sourceLayout);
        if(it == m_pointItems.end())
            continue;

        const auto &items = it.value();
        for(int i = 0; i < items.size(); ++i)
            if(items[i]->isSelected())
                result << qMakePair(sourceLayout, i);
    }
    return result;
}

void PixelLayoutScene::setViewport(QPointF t_scale, QPointF t_offset)
{
    m_scale = t_scale;
    m_offset = t_offset;
    m_inverseScale = QPointF{1.0/m_scale.x(), 1.0/m_scale.y()};

    for(auto &items : m_pointItems)
        for(auto *item : items)
            item->reposition();
}

void PixelLayoutScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsScene::drawBackground(painter, rect);

    // The view always keeps this scene square (see PixelLayoutView::
    // resizeEvent()), letterboxed within a non-square panel - fill outside
    // that square darker so the actual [0,1]x[0,1] edit area's bounds are
    // visible rather than blending into the letterbox margins.
    painter->fillRect(rect, Qt::darkGray);
    painter->fillRect(QRectF(m_offset.x(), m_offset.y(), m_scale.x(), m_scale.y()), Qt::gray);
}

PixelLayoutView::PixelLayoutView(): QGraphicsView()
{
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(QGraphicsView::RubberBandDrag);
}
void PixelLayoutView::paintEvent(QPaintEvent *event)
{
    QGraphicsView::paintEvent(event);
}


void PixelLayoutView::resizeEvent(QResizeEvent *t_event)
{
    QWidget::resizeEvent(t_event);

    double w = t_event->size().width();
    double h = t_event->size().height();

    // The pixel-layout edit area (PixelSourceLayout::pixelBounds(), a
    // normalized [0,1]x[0,1] square) must always render as a true square,
    // not stretched to whatever aspect ratio the panel happens to be - so
    // both axes share one scale factor sized to the shorter side, and the
    // square is centered in the remaining space (letterboxed on the longer
    // axis) rather than anchored to a corner. Centering is done as an
    // explicit scene-space offset baked into point positioning (see
    // PixelLayoutScene::setViewport()) rather than a view transform/scroll -
    // resetTransform() is still needed since the view otherwise keeps
    // whatever transform a previous resize left behind.
    double size = qMin(w, h);
    QPointF offset((w - size) / 2.0, (h - size) / 2.0);

    scene()->setSceneRect(QRect(0,0,w,h));
    resetTransform();
    static_cast<PixelLayoutScene*>(scene())->setViewport(QPointF{size,size}, offset);
}


PixelPointItem::PixelPointItem(PixelSourceLayout *t_sourceLayout, int t_index)
    : QGraphicsItem(), m_sourceLayout(t_sourceLayout), m_index(t_index)
{
    // Selectable/movable is granted per-source by setInteractive(), driven by
    // PixelLayoutScene::selectSource() - only the active source's points can
    // be picked at all, so a rubber-band drag across the whole canvas can
    // never touch another fixture's points.
    setFlags(ItemSendsScenePositionChanges);
}

void PixelPointItem::setInteractive(bool t_interactive)
{
    setFlag(ItemIsSelectable, t_interactive);
    setFlag(ItemIsMovable, t_interactive);
    if(!t_interactive)
        setSelected(false);
}

void PixelPointItem::reposition()
{
    auto *layoutScene = static_cast<PixelLayoutScene*>(scene());
    QPointF sceneScale = layoutScene ? layoutScene->scale() : QPointF{1,1};
    QPointF sceneOffset = layoutScene ? layoutScene->offset() : QPointF{0,0};

    QPointF pos = m_sourceLayout->pixelPosition(m_index);

    m_repositioning = true;
    setPos(sceneOffset.x() + pos.x() * sceneScale.x(), sceneOffset.y() + pos.y() * sceneScale.y());
    m_repositioning = false;
}

QRectF PixelPointItem::boundingRect() const
{
    return QRectF(-4, -4, 8, 8);
}

void PixelPointItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setRenderHint(QPainter::Antialiasing);

    auto *layoutScene = static_cast<PixelLayoutScene*>(scene());
    bool active = layoutScene && layoutScene->activeSourceLayout() == m_sourceLayout;

    painter->setPen(isSelected() ? QPen(Qt::yellow, 1) : Qt::NoPen);
    painter->setBrush(active ? Qt::cyan : Qt::darkGray);
    painter->drawRect(boundingRect());
}

QVariant PixelPointItem::itemChange(GraphicsItemChange t_change, const QVariant &t_value)
{
    // Live visual clamp while dragging, so the point never visibly leaves
    // the edit area mid-drag (PixelSourceLayout::setPixelPosition() below
    // clamps too - that's the authoritative enforcement for every write
    // path, including programmatic ones this never sees - but doing it here
    // as well keeps the drag itself from looking like it detaches from the
    // cursor at the boundary).
    if(t_change == ItemPositionChange && scene() && !m_repositioning)
    {
        auto *layoutScene = static_cast<PixelLayoutScene*>(scene());
        QRectF bounds = PixelSourceLayout::pixelBounds();
        QPointF sceneOffset = layoutScene->offset();
        QRectF sceneBounds(sceneOffset.x() + bounds.x() * layoutScene->scale().x(), sceneOffset.y() + bounds.y() * layoutScene->scale().y(),
                            bounds.width() * layoutScene->scale().x(), bounds.height() * layoutScene->scale().y());

        QPointF newPos = t_value.toPointF();
        if(!sceneBounds.contains(newPos))
        {
            newPos.setX(qBound(sceneBounds.left(), newPos.x(), sceneBounds.right()));
            newPos.setY(qBound(sceneBounds.top(), newPos.y(), sceneBounds.bottom()));
            return newPos;
        }
    }

    if(t_change == ItemPositionHasChanged && !m_repositioning)
    {
        auto *layoutScene = static_cast<PixelLayoutScene*>(scene());
        QPointF sceneScale = layoutScene ? layoutScene->scale() : QPointF{1,1};
        QPointF sceneOffset = layoutScene ? layoutScene->offset() : QPointF{0,0};

        QPointF scenePos = t_value.toPointF();
        QPointF pos((scenePos.x() - sceneOffset.x()) / sceneScale.x(), (scenePos.y() - sceneOffset.y()) / sceneScale.y());

        m_sourceLayout->setPixelPosition(m_index, pos);
    }
    return QGraphicsItem::itemChange(t_change, t_value);
}

PixelSourceListWidget::PixelSourceListWidget(QWidget *t_parent) : QListWidget(t_parent)
{
    setAcceptDrops(true);
}

void PixelSourceListWidget::dragEnterEvent(QDragEnterEvent *t_event)
{
    if(t_event->mimeData()->hasFormat(SceneObject::SceneObjectMime))
        t_event->acceptProposedAction();
    else
        QListWidget::dragEnterEvent(t_event);
}

void PixelSourceListWidget::dragMoveEvent(QDragMoveEvent *t_event)
{
    if(t_event->mimeData()->hasFormat(SceneObject::SceneObjectMime))
        t_event->acceptProposedAction();
    else
        QListWidget::dragMoveEvent(t_event);
}

void PixelSourceListWidget::dropEvent(QDropEvent *t_event)
{
    if(t_event->mimeData()->hasFormat(SceneObject::SceneObjectMime))
    {
        emit sceneObjectsDropped(decodeSceneObjectMime(t_event->mimeData()));
        t_event->acceptProposedAction();
    }
    else
        QListWidget::dropEvent(t_event);
}

PixelLayoutEditorSidePanel::PixelLayoutEditorSidePanel(PixelLayout *t_layout) : QWidget(),pixelLayout(t_layout)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    scene = new PixelLayoutScene(t_layout);

    view = new PixelLayoutView();
    view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    view->setScene(scene);
    view->setFrameRect(QRect(0,0,1,1));
    //scene->setScale(500);
    //view->setTransform(QTransform::fromScale(500,500));
    scene->setSceneRect(QRect(0,0,1,1));

    addButton = new QPushButton("Add");
    removeButton = new QPushButton("Remove");
    removeButton->setEnabled(false);
    arrangeButton = new QPushButton("Arrange");
    arrangeButton->setEnabled(false);
    layoutList = new PixelSourceListWidget;

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(removeButton);
    buttonLayout->addWidget(arrangeButton);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->addLayout(buttonLayout);
    vLayout->addWidget(layoutList);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(view);
    hLayout->addLayout(vLayout);


    setLayout(hLayout);

    connect(layoutList, &QListWidget::currentRowChanged, this, &PixelLayoutEditorSidePanel::selectedRow);
    connect(addButton, &QPushButton::clicked, this, &PixelLayoutEditorSidePanel::addClicked);
    connect(removeButton, &QPushButton::clicked, this, &PixelLayoutEditorSidePanel::removeClicked);
    connect(arrangeButton, &QPushButton::clicked, this, &PixelLayoutEditorSidePanel::arrangeClicked);
    connect(layoutList, &PixelSourceListWidget::sceneObjectsDropped, this, &PixelLayoutEditorSidePanel::sceneObjectsDropped);
    // Individual pixels are selected directly in the view (rubber-band or
    // click), independent of which fixture/strip row is current - the
    // Arrange button only makes sense once at least one is selected.
    connect(scene, &QGraphicsScene::selectionChanged, this, [this](){
        arrangeButton->setEnabled(!scene->selectedItems().isEmpty());
    });

    auto sources = pixelLayout->sources();
    for(auto src : sources)
    {

        auto sourceObj = dynamic_cast<SceneObject*>(src);
        if(dynamic_cast<FixturePixelSource*>(src))
        {
            layoutList->addItem(static_cast<FixturePixelSource*>(src)->fixture()->name());
        }
        else if(sourceObj)
            layoutList->addItem(src ? sourceObj->name() : "Unnamed");
    }

}

void PixelLayoutEditorSidePanel::selectedRow(int t_row)
{
    bool hasSelection = t_row >= 0 && t_row < pixelLayout->sourceLayouts().length();
    removeButton->setEnabled(hasSelection);

    auto selected = hasSelection ? pixelLayout->sourceLayouts()[t_row] : nullptr;
    scene->selectSource(selected);
}

void PixelLayoutEditorSidePanel::removeClicked()
{
    int row = layoutList->currentRow();
    if(row < 0 || row >= pixelLayout->sourceLayouts().length())
        return;

    auto sourceLayout = pixelLayout->sourceLayouts()[row];
    auto source = sourceLayout->source();

    pixelLayout->removeSource(sourceLayout);
    delete layoutList->takeItem(row);

    // The layout owns neither the wrapper nor (for a fixture) the PixelSource
    // it wraps - both were allocated fresh by tryAddSceneObject()/addSource()
    // and nothing else references them, so removal is also where they're
    // freed. A PixelSource that's itself a SceneObject (e.g. a PixelStrip) is
    // owned by the scene, not by this list, and must be left alone.
    delete sourceLayout;
    if(dynamic_cast<FixturePixelSource*>(source))
        delete source;

    // Qt only emits currentRowChanged() when the *row number* changes - if
    // the removed row wasn't the last one, the item that shifts into its
    // place keeps the same row number as current, so the signal never fires
    // even though which source it points to just changed. Re-sync explicitly
    // rather than relying on it, or the view keeps the old (now-deleted)
    // source active until some other selection change happens to fire it.
    selectedRow(layoutList->currentRow());
}

namespace {

QPointF selectionCentroid(const QVector<QPair<PixelSourceLayout*,int>> &t_selection)
{
    QPointF centroid;
    for(const auto &entry : t_selection)
        centroid += entry.first->pixelPosition(entry.second);
    return t_selection.isEmpty() ? centroid : centroid / t_selection.size();
}

} // namespace

void PixelLayoutEditorSidePanel::arrangeClicked()
{
    QMenu menu;
    menu.addAction("Linear...", this, &PixelLayoutEditorSidePanel::arrangeLinear);
    menu.addAction("Grid...", this, &PixelLayoutEditorSidePanel::arrangeGrid);
    menu.addAction("Radial...", this, &PixelLayoutEditorSidePanel::arrangeRadial);
    menu.addAction("Arc...", this, &PixelLayoutEditorSidePanel::arrangeArc);
    menu.addAction("Honeycomb...", this, &PixelLayoutEditorSidePanel::arrangeHoneycomb);
    menu.addAction("Bee Eye...", this, &PixelLayoutEditorSidePanel::arrangeBeeEye);
    menu.addSeparator();
    menu.addAction("Move...", this, &PixelLayoutEditorSidePanel::arrangeMove);
    menu.addAction("Scale...", this, &PixelLayoutEditorSidePanel::arrangeScale);
    menu.addAction("Rotate...", this, &PixelLayoutEditorSidePanel::arrangeRotate);
    menu.exec(arrangeButton->mapToGlobal(QPoint{}));
}

void PixelLayoutEditorSidePanel::arrangeLinear()
{
    int count = scene->selectedPixels().size();
    if(count == 0)
        return;

    LinearArrangeDialog dialog(this);
    if(dialog.exec() != QDialog::Accepted)
        return;

    commitArrangedPoints(PixelArrange::linear(count, dialog.length(), dialog.center(), dialog.angle()));
}

void PixelLayoutEditorSidePanel::arrangeGrid()
{
    int count = scene->selectedPixels().size();
    if(count == 0)
        return;

    GridArrangeDialog dialog(this);
    if(dialog.exec() != QDialog::Accepted)
        return;

    commitArrangedPoints(PixelArrange::grid(count, dialog.rows(), dialog.columns(), dialog.width(), dialog.height()));
}

void PixelLayoutEditorSidePanel::arrangeRadial()
{
    int count = scene->selectedPixels().size();
    if(count == 0)
        return;

    RadialArrangeDialog dialog(this);
    if(dialog.exec() != QDialog::Accepted)
        return;

    commitArrangedPoints(PixelArrange::radial(count, dialog.radius()));
}

void PixelLayoutEditorSidePanel::arrangeArc()
{
    int count = scene->selectedPixels().size();
    if(count == 0)
        return;

    ArcArrangeDialog dialog(this);
    if(dialog.exec() != QDialog::Accepted)
        return;

    commitArrangedPoints(PixelArrange::arc(count, dialog.radius(), dialog.startAngle(), dialog.sweepAngle()));
}

void PixelLayoutEditorSidePanel::arrangeHoneycomb()
{
    int count = scene->selectedPixels().size();
    if(count == 0)
        return;

    HoneycombArrangeDialog dialog(this);
    if(dialog.exec() != QDialog::Accepted)
        return;

    commitArrangedPoints(PixelArrange::honeycomb(count, dialog.rows(), dialog.columns(), dialog.spacing()));
}

void PixelLayoutEditorSidePanel::arrangeBeeEye()
{
    int count = scene->selectedPixels().size();
    if(count == 0)
        return;

    BeeEyeArrangeDialog dialog(this);
    if(dialog.exec() != QDialog::Accepted)
        return;

    commitArrangedPoints(PixelArrange::beeEye(count, dialog.spacing()));
}

void PixelLayoutEditorSidePanel::arrangeMove()
{
    if(scene->selectedPixels().isEmpty())
        return;

    MoveArrangeDialog dialog(this);
    if(dialog.exec() != QDialog::Accepted)
        return;

    QPointF offset(dialog.dx(), dialog.dy());
    transformSelected([offset](QPointF t_pos){ return t_pos + offset; });
}

void PixelLayoutEditorSidePanel::arrangeScale()
{
    auto selection = scene->selectedPixels();
    if(selection.isEmpty())
        return;

    ScaleArrangeDialog dialog(this);
    if(dialog.exec() != QDialog::Accepted)
        return;

    QPointF centroid = selectionCentroid(selection);
    double sx = dialog.scaleX();
    double sy = dialog.scaleY();
    transformSelected([centroid, sx, sy](QPointF t_pos){
        return QPointF(centroid.x() + (t_pos.x() - centroid.x()) * sx,
                        centroid.y() + (t_pos.y() - centroid.y()) * sy);
    });
}

void PixelLayoutEditorSidePanel::arrangeRotate()
{
    auto selection = scene->selectedPixels();
    if(selection.isEmpty())
        return;

    RotateArrangeDialog dialog(this);
    if(dialog.exec() != QDialog::Accepted)
        return;

    QPointF centroid = selectionCentroid(selection);
    QTransform t;
    t.translate(centroid.x(), centroid.y());
    t.rotate(dialog.angle());
    t.translate(-centroid.x(), -centroid.y());
    transformSelected([t](QPointF t_pos){ return t.map(t_pos); });
}

void PixelLayoutEditorSidePanel::transformSelected(const std::function<QPointF(QPointF)> &t_fn)
{
    auto selection = scene->selectedPixels();
    for(const auto &entry : selection)
        entry.first->setPixelPosition(entry.second, t_fn(entry.first->pixelPosition(entry.second)));
}

void PixelLayoutEditorSidePanel::commitArrangedPoints(const QVector<QPointF> &t_shapePoints)
{
    auto selection = scene->selectedPixels();
    if(selection.size() != t_shapePoints.size())
        return;

    // Anchor the shape on the selection's current centroid, rather than
    // wherever PixelArrange:: happens to center it locally - "Arrange"
    // replaces the selected points roughly where they already are.
    QPointF fromCentroid = selectionCentroid(selection);

    QPointF shapeCentroid;
    for(const auto &pt : t_shapePoints)
        shapeCentroid += pt;
    shapeCentroid /= t_shapePoints.size();

    for(int i = 0; i < selection.size(); ++i)
        selection[i].first->setPixelPosition(selection[i].second, t_shapePoints[i] - shapeCentroid + fromCentroid);
}

// A fixture counts as "added" if any current FixturePixelSource wraps that
// same fixture (the PixelSource itself is only constructed on add, so
// comparing PixelSource pointers - as a plain PixelSource-derived
// SceneObject would - can't detect that case).
bool PixelLayoutEditorSidePanel::isAlreadyAdded(SceneObject *t_obj) const
{
    auto currentSources = pixelLayout->sources();

    if(auto fixture = dynamic_cast<Fixture*>(t_obj))
    {
        for(auto existing : currentSources)
        {
            auto fixtureSource = dynamic_cast<FixturePixelSource*>(existing);
            if(fixtureSource && fixtureSource->fixture() == fixture)
                return true;
        }
        return false;
    }

    return currentSources.contains(dynamic_cast<PixelSource*>(t_obj));
}

void PixelLayoutEditorSidePanel::tryAddSceneObject(SceneObject *t_obj)
{
    if(!t_obj || isAlreadyAdded(t_obj))
        return;

    if(auto fixture = dynamic_cast<Fixture*>(t_obj))
    {
        auto capabilities = fixture->findCapability(Capability_Color);
        if(!capabilities.isEmpty())
            addSource(new FixturePixelSource(capabilities));
        return;
    }

    if(auto source = dynamic_cast<PixelSource*>(t_obj))
        addSource(source);
}

void PixelLayoutEditorSidePanel::sceneObjectsDropped(const QVector<SceneObject*> &t_objects)
{
    for(auto obj : t_objects)
        tryAddSceneObject(obj);
}

void PixelLayoutEditorSidePanel::addClicked()
{
    auto sources = SceneIterator::FindMany(photonApp->project()->sceneRoot(),[](SceneObject *obj, bool *keepGoing){
            *keepGoing = true;
        auto fixture = dynamic_cast<Fixture*>(obj);

        if(fixture)
        {
            if(!fixture->findCapability(Capability_Color).isEmpty()){
                    return dynamic_cast<SceneObject*>(obj);
                }
        }
        else
            {
            if(dynamic_cast<PixelSource*>(obj))
                return dynamic_cast<SceneObject*>(obj);
        }
        return static_cast<SceneObject*>(nullptr);

    });

    QMenu menu;
    for(auto src : sources)
    {
        auto action = menu.addAction(src->name(),[src, this](){
            tryAddSceneObject(src);
        });
        action->setEnabled(!isAlreadyAdded(src));
    }

    menu.exec(addButton->mapToGlobal(QPoint{}));

}

void PixelLayoutEditorSidePanel::addSource(photon::PixelSource *t_source)
{
    auto sourceLayout = new PixelSourceLayout(t_source);
    pixelLayout->addSource(sourceLayout);

    auto sourceObj = dynamic_cast<SceneObject*>(t_source);

    if(dynamic_cast<FixturePixelSource*>(t_source))
    {
        layoutList->addItem(static_cast<FixturePixelSource*>(t_source)->fixture()->name());
    }
    else
        layoutList->addItem(sourceObj ? sourceObj->name() : "Unnamed");
}

PixelLayoutEditor::PixelLayoutEditor(PixelLayout *t_layout, QWidget *t_parent) : QWidget(t_parent),m_impl(new Impl)
{
    // Opts into PropertiesPanel giving this widget its trailing stretch
    // instead of leaving it capped at its sizeHint - the pixel-mapping
    // canvas should grow to use whatever room the panel has, not sit in a
    // small fixed square with dead space below it.
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_impl->sidePanel = new PixelLayoutEditorSidePanel(t_layout);


    auto hLayout = new QHBoxLayout;
    hLayout->addWidget(m_impl->sidePanel);

    setLayout(hLayout);
}

PixelLayoutEditor::~PixelLayoutEditor()
{
    delete m_impl;
}

PixelLayout *PixelLayoutEditor::pixelLayout() const
{
    return m_impl->pixelLayout;
}

} // namespace photon
