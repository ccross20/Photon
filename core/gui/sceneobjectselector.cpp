#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QSet>
#include "sceneobjectselector_p.h"
#include "scene/sceneobject.h"

namespace photon {

// ─────────────────────────────────────────────────────────────────────────────
// SelectorItemDelegate
// ─────────────────────────────────────────────────────────────────────────────

SelectorItemDelegate::SelectorItemDelegate(QObject *parent) : QAbstractItemDelegate(parent)
{
}

bool SelectorItemDelegate::editorEvent(QEvent *event, QAbstractItemModel *model,
                                       const QStyleOptionViewItem &,
                                       const QModelIndex &index)
{
    if (event->type() != QEvent::MouseButtonPress)
        return false;

    const QModelIndex col0 = index.sibling(index.row(), 0);
    const int currentState = model->data(col0, Qt::UserRole).toInt();
    model->setData(col0, currentState == 1 ? 0 : 1);
    return true;
}

void SelectorItemDelegate::paint(QPainter *painter,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const int     state = index.data(Qt::UserRole).toInt();
    const QString name  = index.data(Qt::DisplayRole).toString();

    // Subtle hover highlight
    if (option.state & QStyle::State_MouseOver)
        painter->fillRect(option.rect, QColor(255, 255, 255, 18));

    // Checkbox — small rounded square at the left of the content rect
    constexpr int cbSize = 14;
    const int cbX = option.rect.left() + 4;
    const int cbY = option.rect.top() + (option.rect.height() - cbSize) / 2;
    const QRect cbRect(cbX, cbY, cbSize, cbSize);

    switch (state) {
    case 1: { // selected — solid green with white checkmark
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(60, 180, 80));
        painter->drawRoundedRect(cbRect, 3, 3);
        painter->setBrush(Qt::NoBrush);
        QPen checkPen(Qt::white, 1.5f, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(checkPen);
        const QPointF pts[3] = {
            { static_cast<qreal>(cbRect.left() + 3),    cbRect.top()  + cbSize * 0.55 },
            { cbRect.left() + cbSize * 0.42,             static_cast<qreal>(cbRect.bottom() - 3) },
            { static_cast<qreal>(cbRect.right() - 2),   static_cast<qreal>(cbRect.top() + 3) }
        };
        painter->drawPolyline(pts, 3);
        break;
    }
    case 2: // ancestor selected — dimmed, non-interactive appearance
        painter->setPen(QPen(QColor(70, 70, 70), 1));
        painter->setBrush(QColor(55, 55, 55));
        painter->drawRoundedRect(cbRect, 3, 3);
        break;
    case 3: { // partial — green-bordered box with a dash (mixed/indeterminate)
        painter->setPen(QPen(QColor(80, 160, 80), 1.5f));
        painter->setBrush(QColor(30, 65, 30));
        painter->drawRoundedRect(cbRect, 3, 3);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(100, 200, 110), 2.0f, Qt::SolidLine, Qt::RoundCap));
        const int midY = cbRect.center().y();
        painter->drawLine(cbRect.left() + 3, midY, cbRect.right() - 3, midY);
        break;
    }
    default: // not selected — empty box with subtle border
        painter->setPen(QPen(QColor(90, 90, 90), 1));
        painter->setBrush(QColor(45, 45, 45));
        painter->drawRoundedRect(cbRect, 3, 3);
        break;
    }

    // Name text — slightly dimmed for ancestor-selected items
    const QRect textRect(cbRect.right() + 6, option.rect.top(),
                         option.rect.right() - cbRect.right() - 8, option.rect.height());
    painter->setPen(state == 2 ? QColor(120, 120, 120) : QColor(210, 210, 210));
    painter->setBrush(Qt::NoBrush);
    painter->setFont(option.font);
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, name);

    painter->restore();
}

QSize SelectorItemDelegate::sizeHint(const QStyleOptionViewItem &,
                                      const QModelIndex &) const
{
    return QSize(100, 24);
}

// ─────────────────────────────────────────────────────────────────────────────
// SelectorModel
// ─────────────────────────────────────────────────────────────────────────────

SelectorModel::SelectorModel(SceneObject *t_root, QObject *t_parent)
    : SceneModel(t_root, t_parent)
{
}

QVariant SelectorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
        return QVariant("Name");
    return QVariant();
}

QVariant SelectorModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !rootObject())
        return QVariant();

    SceneObject *obj = objectForIndex(index);
    if (!obj)
        return QVariant();

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (index.column() == 0)
            return obj->name();
        break;

    case Qt::UserRole:
        if (index.column() == 0) {
            if (hasAncestorSelected(obj))        return 2;
            if (m_selectedObjects.contains(obj)) return 1;
            if (hasSomeDescendantSelected(obj))  return 3;
            return 0;
        }
        break;

    case Qt::ToolTipRole:
        return QVariant(obj->name());

    case Qt::SizeHintRole:
        return QSize(100, 24);
    }

    return QVariant();
}

bool SelectorModel::hasAncestorSelected(SceneObject *t_obj) const
{
    SceneObject *par = t_obj->parentSceneObject();
    while (par) {
        if (m_selectedObjects.contains(par))
            return true;
        par = par->parentSceneObject();
    }
    return false;
}

bool SelectorModel::hasSomeDescendantSelected(SceneObject *t_obj) const
{
    for (SceneObject *child : t_obj->sceneChildren()) {
        if (m_selectedObjects.contains(child))
            return true;
        if (hasSomeDescendantSelected(child))
            return true;
    }
    return false;
}

int SelectorModel::columnCount(const QModelIndex &) const
{
    return 1;
}

bool SelectorModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;
    if (index.column() != 0)
        return false;
    if (hasAncestorSelected(objectForIndex(index)))
        return false;

    if (value.toBool())
        m_selectedObjects.append(objectForIndex(index));
    else
        m_selectedObjects.removeOne(objectForIndex(index));

    sortByTreeOrder();
    emitDataChangedForAll(QModelIndex()); // update all so parent partial-state refreshes too
    emit selectionChanged();
    return true;
}

void SelectorModel::sortByTreeOrder()
{
    const QSet<SceneObject *> selected(m_selectedObjects.cbegin(), m_selectedObjects.cend());
    m_selectedObjects.clear();
    collectInTreeOrder(QModelIndex(), selected, m_selectedObjects);
}

void SelectorModel::collectInTreeOrder(const QModelIndex &parent,
                                       const QSet<SceneObject *> &selected,
                                       QVector<SceneObject *> &result) const
{
    const int count = rowCount(parent);
    for (int i = 0; i < count; ++i) {
        const QModelIndex idx = index(i, 0, parent);
        SceneObject *obj = objectForIndex(idx);
        if (obj && selected.contains(obj))
            result.append(obj);
        collectInTreeOrder(idx, selected, result);
    }
}

void SelectorModel::collectAll(const QModelIndex &parent, QVector<SceneObject *> &result) const
{
    const int count = rowCount(parent);
    for (int i = 0; i < count; ++i) {
        const QModelIndex idx = index(i, 0, parent);
        SceneObject *obj = objectForIndex(idx);
        if (obj)
            result.append(obj);
        collectAll(idx, result);
    }
}

void SelectorModel::emitDataChangedForAll(const QModelIndex &parent)
{
    const int rows = rowCount(parent);
    if (rows == 0)
        return;
    emit dataChanged(index(0, 0, parent), index(rows - 1, columnCount() - 1, parent));
    for (int i = 0; i < rows; ++i)
        emitDataChangedForAll(index(i, 0, parent));
}

void SelectorModel::setSelectedObjects(const QVector<SceneObject *> &t_selected)
{
    m_selectedObjects = t_selected;
    emitDataChangedForAll(QModelIndex());
}

const QVector<SceneObject *> &SelectorModel::selectedObjects() const
{
    return m_selectedObjects;
}

void SelectorModel::selectAll()
{
    m_selectedObjects.clear();
    collectAll(QModelIndex(), m_selectedObjects);
    emitDataChangedForAll(QModelIndex());
    emit selectionChanged();
}

void SelectorModel::selectNone()
{
    m_selectedObjects.clear();
    emitDataChangedForAll(QModelIndex());
    emit selectionChanged();
}

// ─────────────────────────────────────────────────────────────────────────────
// SceneObjectSelector
// ─────────────────────────────────────────────────────────────────────────────

SceneObjectSelector::Impl::Impl(SceneObjectSelector *t_facade, SceneObject *t_root)
    : facade(t_facade)
{
    model = new SelectorModel(t_root, facade);

    view = new QTreeView;
    view->setModel(model);
    view->setItemDelegate(new SelectorItemDelegate(view));
    view->setHeaderHidden(true);
    view->setSelectionMode(QAbstractItemView::NoSelection);
    view->setMouseTracking(true);
    view->viewport()->setMouseTracking(true);
    view->setUniformRowHeights(true);
    view->setIndentation(16);
    view->expandAll();

    selectAllButton  = new QPushButton("Select All");
    selectNoneButton = new QPushButton("Select None");
}

SceneObjectSelector::SceneObjectSelector(SceneObject *t_root, QWidget *parent)
    : QWidget{parent}, m_impl(new Impl(this, t_root))
{
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addWidget(m_impl->selectAllButton);
    buttonLayout->addWidget(m_impl->selectNoneButton);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->addLayout(buttonLayout);
    vLayout->addWidget(m_impl->view);
    setLayout(vLayout);

    connect(m_impl->model, &SelectorModel::selectionChanged,
            this, &SceneObjectSelector::selectionChanged);
    connect(m_impl->selectAllButton,  &QPushButton::clicked,
            m_impl->model, &SelectorModel::selectAll);
    connect(m_impl->selectNoneButton, &QPushButton::clicked,
            m_impl->model, &SelectorModel::selectNone);
}

SceneObjectSelector::~SceneObjectSelector()
{
    delete m_impl;
}

void SceneObjectSelector::setSelectedObjects(const QVector<SceneObject *> &t_selected)
{
    m_impl->model->setSelectedObjects(t_selected);
}

QVector<SceneObject *> SceneObjectSelector::selectedObjects() const
{
    return m_impl->model->selectedObjects();
}

} // namespace photon
