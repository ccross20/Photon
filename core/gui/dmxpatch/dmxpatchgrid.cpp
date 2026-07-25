#include <algorithm>
#include <QPainter>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDrag>
#include <QDataStream>
#include <QCoreApplication>
#include <QApplication>
#include "dmxpatchgrid.h"
#include "fixture/fixture.h"
#include "fixture/fixturecollection.h"
#include "project/project.h"

namespace photon {

namespace {
QColor colorForFixture(const QByteArray &t_uniqueId)
{
    const int hue = int(qHash(t_uniqueId) % 360u);
    return QColor::fromHsv(hue, 150, 200);
}
}

class DMXPatchGrid::Impl
{
public:
    Project *project = nullptr;
    int universe = 1;

    const int columns = 32;
    const int rows = 16;
    const qreal cellWidth = 28;
    const qreal cellHeight = 34;
    const qreal margin = 6;

    // The 512 cells + channel numbers never change once drawn, so they're
    // rendered once into this and blitted each paintEvent — repainting during
    // a drag would otherwise redraw 512 individual rects/text labels a frame.
    QPixmap backgroundPixmap;

    // Cached from the project; refreshed only on refreshFixtures(), not on
    // every paint — Project::fixtures() walks the whole scene tree, which is
    // too costly to redo on every repaint while dragging.
    QVector<Fixture*> allFixtures;

    Fixture *pressedFixture = nullptr;
    QPoint pressPos;
    bool dragging = false;
    // Set when a plain (unmodified) click lands on a fixture that's already
    // part of the current multi-selection — collapsing to just that fixture
    // is deferred to mouseReleaseEvent so the whole selection is still intact
    // (and draggable as a group) if this turns into a drag instead of a click.
    bool deferClickCollapse = false;

    // Ordered by selection; the last entry is the "primary" (most recently
    // clicked/toggled-on) fixture, which is what gets pushed to the project
    // as its single selectedSceneObject for cross-panel sync.
    QVector<Fixture*> selectedFixtures;

    // Fixtures currently being dragged over the grid (decoded once, in
    // dragEnterEvent) — drives the ghosted full-bar preview.
    QVector<Fixture*> draggingFixtures;
    bool dropPreviewActive = false;
    int dropPreviewChannel = -1;

    // Set while pushing our own selection out to the project, so the
    // resulting Project::selectedSceneObjectsChanged echo (which the panel
    // forwards straight back into setSelectedFixtures) is a no-op instead of
    // redundantly re-applying the selection we just set.
    bool syncingSelection = false;

    int channelCount() const { return columns * rows; }
};

DMXPatchGrid::DMXPatchGrid(QWidget *parent) : QWidget(parent), m_impl(new Impl)
{
    setAcceptDrops(true);
    setMouseTracking(true);
    // Not placed in a layout when hosted in a QScrollArea (setWidget() rather
    // than a layout), so it won't otherwise pick up sizeHint() on its own —
    // and since this is a fixed 512-channel grid, a fixed size is correct.
    setFixedSize(sizeHint());
    rebuildBackground();
}

DMXPatchGrid::~DMXPatchGrid()
{
    delete m_impl;
}

int DMXPatchGrid::universe() const
{
    return m_impl->universe;
}

void DMXPatchGrid::setUniverse(int t_universe)
{
    if(m_impl->universe == t_universe)
        return;
    m_impl->universe = t_universe;
    update();
}

void DMXPatchGrid::setProject(Project *t_project)
{
    m_impl->project = t_project;
    refreshFixtures();
}

void DMXPatchGrid::refreshFixtures()
{
    m_impl->allFixtures.clear();
    if(m_impl->project)
    {
        for(auto *fixture : m_impl->project->fixtures()->fixtures())
            m_impl->allFixtures.append(fixture);
    }
    update();
}

void DMXPatchGrid::setSelectedFixtures(const QList<SceneObject*> &t_objects)
{
    if(m_impl->syncingSelection)
        return;

    QVector<Fixture*> fixtures;
    for(auto *obj : t_objects)
    {
        if(auto *fixture = dynamic_cast<Fixture*>(obj))
            fixtures.append(fixture);
    }

    if(fixtures == m_impl->selectedFixtures)
        return;

    m_impl->selectedFixtures = fixtures;
    update();
}

Fixture *DMXPatchGrid::selectedFixture() const
{
    return m_impl->selectedFixtures.isEmpty() ? nullptr : m_impl->selectedFixtures.last();
}

QVector<Fixture*> DMXPatchGrid::selectedFixtures() const
{
    return m_impl->selectedFixtures;
}

bool DMXPatchGrid::isSelected(Fixture *t_fixture) const
{
    return t_fixture && m_impl->selectedFixtures.contains(t_fixture);
}

void DMXPatchGrid::replaceSelection(Fixture *t_fixture)
{
    m_impl->selectedFixtures.clear();
    if(t_fixture)
        m_impl->selectedFixtures.append(t_fixture);
    pushPrimarySelectionToProject();
    update();
}

void DMXPatchGrid::addToSelection(Fixture *t_fixture)
{
    if(!t_fixture)
        return;
    m_impl->selectedFixtures.removeAll(t_fixture);
    m_impl->selectedFixtures.append(t_fixture);
    pushPrimarySelectionToProject();
    update();
}

void DMXPatchGrid::toggleSelection(Fixture *t_fixture)
{
    if(!t_fixture)
        return;
    if(m_impl->selectedFixtures.removeAll(t_fixture) == 0)
        m_impl->selectedFixtures.append(t_fixture);
    pushPrimarySelectionToProject();
    update();
}

void DMXPatchGrid::pushPrimarySelectionToProject()
{
    if(!m_impl->project)
        return;

    QList<SceneObject*> objects;
    for(auto *fixture : m_impl->selectedFixtures)
        objects.append(fixture);

    m_impl->syncingSelection = true;
    m_impl->project->setSelectedSceneObjects(objects);
    m_impl->syncingSelection = false;
}

QSize DMXPatchGrid::sizeHint() const
{
    return QSize(int(m_impl->margin * 2 + m_impl->columns * m_impl->cellWidth),
                 int(m_impl->margin * 2 + m_impl->rows * m_impl->cellHeight));
}

void DMXPatchGrid::rebuildBackground()
{
    m_impl->backgroundPixmap = QPixmap(size());
    m_impl->backgroundPixmap.fill(palette().window().color());

    QPainter painter(&m_impl->backgroundPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont channelFont = font();
    channelFont.setPointSizeF(qMax(6.0, font().pointSizeF() - 3.0));
    painter.setFont(channelFont);

    for(int channel = 0; channel < m_impl->channelCount(); ++channel)
    {
        const QRectF r = cellRect(channel);
        painter.setPen(QColor(60, 60, 60));
        painter.setBrush(QColor(40, 40, 40));
        painter.drawRect(r);
        painter.setPen(QColor(130, 130, 130));
        painter.drawText(r.adjusted(3, 2, -2, -2), Qt::AlignTop | Qt::AlignLeft, QString::number(channel + 1));
    }
}

QRectF DMXPatchGrid::cellRect(int t_channel) const
{
    const int col = t_channel % m_impl->columns;
    const int row = t_channel / m_impl->columns;
    return QRectF(m_impl->margin + col * m_impl->cellWidth, m_impl->margin + row * m_impl->cellHeight,
                  m_impl->cellWidth, m_impl->cellHeight);
}

int DMXPatchGrid::channelAt(const QPointF &t_pos) const
{
    int col = int((t_pos.x() - m_impl->margin) / m_impl->cellWidth);
    int row = int((t_pos.y() - m_impl->margin) / m_impl->cellHeight);
    col = qBound(0, col, m_impl->columns - 1);
    row = qBound(0, row, m_impl->rows - 1);
    return row * m_impl->columns + col;
}

QVector<DMXPatchGrid::Segment> DMXPatchGrid::segmentsFor(int t_offset, int t_size) const
{
    QVector<Segment> segments;
    if(t_size <= 0)
        return segments;

    const int last = qBound(0, t_offset + t_size - 1, m_impl->channelCount() - 1);
    int c = qBound(0, t_offset, m_impl->channelCount() - 1);
    if(c > last)
        return segments;

    while(c <= last)
    {
        const int row = c / m_impl->columns;
        const int rowEnd = row * m_impl->columns + m_impl->columns - 1;
        const int segEnd = qMin(last, rowEnd);
        segments.append({row, c % m_impl->columns, segEnd % m_impl->columns});
        c = segEnd + 1;
    }
    return segments;
}

// The full bar rect for one segment, in widget coordinates.
QRectF DMXPatchGrid::segmentRect(const Segment &t_segment) const
{
    const QRectF first = cellRect(t_segment.row * m_impl->columns + t_segment.colStart);
    const QRectF last = cellRect(t_segment.row * m_impl->columns + t_segment.colEnd);
    return QRectF(first.left(), first.top(), last.right() - first.left(), first.height());
}

QRectF DMXPatchGrid::laneRect(const Segment &t_segment, int t_lane, int t_maxLanes) const
{
    const QRectF full = segmentRect(t_segment);
    if(t_maxLanes <= 1)
        return full;

    const qreal laneHeight = full.height() / t_maxLanes;
    return QRectF(full.left(), full.top() + t_lane * laneHeight, full.width(), laneHeight);
}

// Classic interval partitioning: sort by start channel, and give each fixture
// the lowest lane whose most-recent occupant already ended before this one
// starts. Fixtures that don't overlap anything end up back in lane 0.
QHash<Fixture*, int> DMXPatchGrid::computeLanes(const QVector<Fixture*> &t_fixtures, int *t_outMaxLanes) const
{
    QVector<Fixture*> sorted = t_fixtures;
    std::sort(sorted.begin(), sorted.end(), [](Fixture *a, Fixture *b) {
        return a->dmxOffset() < b->dmxOffset();
    });

    QVector<int> laneEnds; // last-used end channel, per lane
    QHash<Fixture*, int> laneOf;

    for(auto *fixture : sorted)
    {
        const int start = fixture->dmxOffset();
        const int end = fixture->dmxOffset() + fixture->dmxSize() - 1;

        int chosenLane = -1;
        for(int i = 0; i < laneEnds.size(); ++i)
        {
            if(laneEnds[i] < start)
            {
                chosenLane = i;
                break;
            }
        }

        if(chosenLane == -1)
        {
            laneEnds.append(end);
            chosenLane = laneEnds.size() - 1;
        }
        else
        {
            laneEnds[chosenLane] = end;
        }
        laneOf[fixture] = chosenLane;
    }

    if(t_outMaxLanes)
        *t_outMaxLanes = qMax(1, laneEnds.size());
    return laneOf;
}

QVector<Fixture*> DMXPatchGrid::fixturesInUniverse() const
{
    QVector<Fixture*> result;
    for(auto *fixture : m_impl->allFixtures)
    {
        if(fixture->universe() == m_impl->universe)
            result.append(fixture);
    }
    return result;
}

Fixture *DMXPatchGrid::fixtureAt(const QPoint &t_pos) const
{
    const auto fixtures = fixturesInUniverse();
    int maxLanes = 1;
    const auto laneOf = computeLanes(fixtures, &maxLanes);

    for(auto *fixture : fixtures)
    {
        const int lane = laneOf.value(fixture, 0);
        for(const auto &segment : segmentsFor(fixture->dmxOffset(), fixture->dmxSize()))
        {
            if(laneRect(segment, lane, maxLanes).contains(t_pos))
                return fixture;
        }
    }
    return nullptr;
}

// Shared by dragEnterEvent (to know what to ghost) and dropEvent (to know
// what to patch) — decoded once per drag rather than per mouse-move.
QVector<Fixture*> DMXPatchGrid::decodeFixtures(const QMimeData *t_mimeData) const
{
    QVector<Fixture*> result;
    if(!t_mimeData->hasFormat(SceneObject::SceneObjectMime))
        return result;

    QByteArray data = t_mimeData->data(SceneObject::SceneObjectMime);
    QDataStream stream(&data, QIODevice::ReadOnly);
    qint64 senderPid;
    stream >> senderPid;
    if(senderPid != QCoreApplication::applicationPid())
        return result;

    int count = 0;
    stream >> count;
    for(int i = 0; i < count; ++i)
    {
        qlonglong ptr;
        stream >> ptr;
        if(auto *fixture = dynamic_cast<Fixture*>(reinterpret_cast<SceneObject*>(ptr)))
            result.append(fixture);
    }
    return result;
}

void DMXPatchGrid::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, m_impl->backgroundPixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Ghosted preview of where the dragged fixture(s) will land, sequenced
    // exactly like dropEvent will actually place them.
    if(m_impl->dropPreviewActive && !m_impl->draggingFixtures.isEmpty())
    {
        int channel = m_impl->dropPreviewChannel;
        QColor ghost(90, 180, 255, 100);
        for(auto *fixture : m_impl->draggingFixtures)
        {
            for(const auto &segment : segmentsFor(channel, fixture->dmxSize()))
            {
                QRectF bar = segmentRect(segment).adjusted(2, 2, -2, -2);
                painter.setPen(QPen(QColor(90, 180, 255), 2, Qt::DashLine));
                painter.setBrush(ghost);
                painter.drawRoundedRect(bar, 4, 4);
            }
            channel += fixture->dmxSize();
        }
    }

    QFontMetrics fm(font());

    const auto fixtures = fixturesInUniverse();
    int maxLanes = 1;
    const auto laneOf = computeLanes(fixtures, &maxLanes);

    for(auto *fixture : fixtures)
    {
        const auto segments = segmentsFor(fixture->dmxOffset(), fixture->dmxSize());
        if(segments.isEmpty())
            continue;

        const int lane = laneOf.value(fixture, 0);
        const bool selected = isSelected(fixture);

        QColor color = colorForFixture(fixture->uniqueId());
        if(m_impl->dragging && selected)
            color.setAlpha(110);

        QRectF firstBar;
        for(int i = 0; i < segments.size(); ++i)
        {
            QRectF bar = laneRect(segments[i], lane, maxLanes).adjusted(2, 2, -2, -2);
            if(i == 0)
                firstBar = bar;
            painter.setPen(QPen(color.darker(160), 1));
            painter.setBrush(color);
            painter.drawRoundedRect(bar, 4, 4);
            if(selected)
            {
                painter.setPen(QPen(QColor(255, 220, 60), 2));
                painter.setBrush(Qt::NoBrush);
                painter.drawRoundedRect(bar, 4, 4);
            }
        }

        painter.setPen(Qt::white);
        const QString label = fm.elidedText(fixture->name(), Qt::ElideRight, int(firstBar.width()) - 6);
        painter.drawText(firstBar.adjusted(4, 0, -2, 0), Qt::AlignVCenter | Qt::AlignLeft, label);
    }
}

void DMXPatchGrid::mousePressEvent(QMouseEvent *event)
{
    m_impl->pressPos = event->pos();
    m_impl->pressedFixture = fixtureAt(event->pos());
    m_impl->dragging = false;
    m_impl->deferClickCollapse = false;

    const auto modifiers = event->modifiers();
    if(modifiers & Qt::ShiftModifier)
    {
        addToSelection(m_impl->pressedFixture);
    }
    else if(modifiers & Qt::ControlModifier)
    {
        toggleSelection(m_impl->pressedFixture);
    }
    else if(!m_impl->pressedFixture || !isSelected(m_impl->pressedFixture))
    {
        replaceSelection(m_impl->pressedFixture);
    }
    else
    {
        // Pressed on a fixture that's already part of a multi-selection —
        // keep the whole selection intact in case this turns into a group
        // drag; only collapse to just this one on release if it doesn't.
        m_impl->deferClickCollapse = true;
    }

    QWidget::mousePressEvent(event);
}

void DMXPatchGrid::mouseMoveEvent(QMouseEvent *event)
{
    if(m_impl->pressedFixture && (event->buttons() & Qt::LeftButton) && !m_impl->dragging
       && (event->pos() - m_impl->pressPos).manhattanLength() >= QApplication::startDragDistance())
    {
        m_impl->dragging = true;
        update();

        // Drag the whole selection as a group, with the fixture actually
        // grabbed landing at the drop point and its selected companions
        // following right after it (dropEvent sequences them in this order).
        QVector<Fixture*> toDrag;
        toDrag.append(m_impl->pressedFixture);
        for(auto *fixture : m_impl->selectedFixtures)
        {
            if(fixture != m_impl->pressedFixture)
                toDrag.append(fixture);
        }

        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << QCoreApplication::applicationPid();
        stream << int(toDrag.size());
        for(auto *fixture : toDrag)
            stream << reinterpret_cast<qlonglong>(static_cast<SceneObject*>(fixture));

        auto *mimeData = new QMimeData;
        mimeData->setData(SceneObject::SceneObjectMime, data);

        auto *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->exec(Qt::MoveAction);

        m_impl->pressedFixture = nullptr;
        m_impl->dragging = false;
        update();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void DMXPatchGrid::mouseReleaseEvent(QMouseEvent *event)
{
    // A deferred plain click (see mousePressEvent) that didn't turn into a
    // drag now collapses the selection down to just the clicked fixture.
    if(!m_impl->dragging && m_impl->deferClickCollapse)
        replaceSelection(m_impl->pressedFixture);

    m_impl->pressedFixture = nullptr;
    m_impl->dragging = false;
    m_impl->deferClickCollapse = false;
    QWidget::mouseReleaseEvent(event);
}

void DMXPatchGrid::dragEnterEvent(QDragEnterEvent *event)
{
    m_impl->draggingFixtures = decodeFixtures(event->mimeData());
    if(m_impl->draggingFixtures.isEmpty())
        return;

    event->acceptProposedAction();
    m_impl->dropPreviewActive = true;
    m_impl->dropPreviewChannel = channelAt(event->position());
    update();
}

void DMXPatchGrid::dragMoveEvent(QDragMoveEvent *event)
{
    if(m_impl->draggingFixtures.isEmpty())
        return;

    event->acceptProposedAction();
    const int channel = channelAt(event->position());
    if(channel == m_impl->dropPreviewChannel)
        return; // same target cell as last move — nothing to redraw
    m_impl->dropPreviewChannel = channel;
    update();
}

void DMXPatchGrid::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_impl->dropPreviewActive = false;
    m_impl->draggingFixtures.clear();
    update();
    QWidget::dragLeaveEvent(event);
}

void DMXPatchGrid::dropEvent(QDropEvent *event)
{
    m_impl->dropPreviewActive = false;
    const QVector<Fixture*> dropped = m_impl->draggingFixtures;
    m_impl->draggingFixtures.clear();
    update();

    if(dropped.isEmpty())
        return;

    // Multiple fixtures dropped together patch back-to-back, in the order
    // they arrived (source selection order), starting at the drop channel.
    int channel = channelAt(event->position());
    for(auto *fixture : dropped)
    {
        fixture->setUniverse(m_impl->universe);
        fixture->setDMXOffset(qBound(0, channel, m_impl->channelCount() - 1));
        channel += fixture->dmxSize();
    }

    event->acceptProposedAction();
}

} // namespace photon
