#include <QWidget>
#include "flowlayout.h"

namespace photon {

FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
    : m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
    QLayoutItem *item;
    while((item = takeAt(0)))
        delete item;
}

void FlowLayout::addItem(QLayoutItem *item)
{
    m_itemList.append(item);
}

int FlowLayout::horizontalSpacing() const
{
    if(m_hSpace >= 0)
        return m_hSpace;
    return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
}

int FlowLayout::verticalSpacing() const
{
    if(m_vSpace >= 0)
        return m_vSpace;
    return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
}

int FlowLayout::count() const
{
    return m_itemList.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
    return m_itemList.value(index);
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    if(index < 0 || index >= m_itemList.size())
        return nullptr;
    return m_itemList.takeAt(index);
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return {};
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int t_width) const
{
    return doLayout(QRect(0, 0, t_width, 0), true);
}

void FlowLayout::setGeometry(const QRect &t_rect)
{
    QLayout::setGeometry(t_rect);
    doLayout(t_rect, false);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for(const QLayoutItem *item : m_itemList)
        size = size.expandedTo(item->minimumSize());

    const QMargins margins = contentsMargins();
    size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    return size;
}

// Lays every item left-to-right within rect's width, wrapping to a new row
// when the next item wouldn't fit. testOnly skips the actual setGeometry()
// calls, since heightForWidth() only needs the resulting total height.
int FlowLayout::doLayout(const QRect &t_rect, bool t_testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = t_rect.adjusted(left, top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    for(QLayoutItem *item : m_itemList)
    {
        const QWidget *widget = item->widget();
        int spaceX = horizontalSpacing();
        if(spaceX == -1)
            spaceX = widget ? widget->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal) : 0;
        int spaceY = verticalSpacing();
        if(spaceY == -1)
            spaceY = widget ? widget->style()->layoutSpacing(
                QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical) : 0;

        int nextX = x + item->sizeHint().width() + spaceX;
        if(nextX - spaceX > effectiveRect.right() && lineHeight > 0)
        {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        if(!t_testOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }

    return y + lineHeight - t_rect.y() + bottom;
}

int FlowLayout::smartSpacing(QStyle::PixelMetric t_pm) const
{
    QObject *parentObj = parent();
    if(!parentObj)
        return -1;

    if(parentObj->isWidgetType())
    {
        auto *pw = static_cast<QWidget*>(parentObj);
        return pw->style()->pixelMetric(t_pm, nullptr, pw);
    }

    return static_cast<QLayout*>(parentObj)->spacing();
}

} // namespace photon
