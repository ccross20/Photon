#include <QApplication>
#include <QContextMenuEvent>
#include <QDrag>
#include <QFontMetrics>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include "tagchip.h"
#include "tagcolor.h"
#include "tagmime.h"

namespace photon {

namespace {
    constexpr int HorizontalPadding = 8;
    constexpr int VerticalPadding = 3;
    constexpr int CornerRadius = 8;
}

QSize tagChipSizeHint(const QFontMetrics &t_metrics, const QString &t_tag)
{
    return QSize(t_metrics.horizontalAdvance(t_tag) + HorizontalPadding * 2,
                t_metrics.height() + VerticalPadding * 2);
}

void paintTagChip(QPainter *t_painter, const QRect &t_rect, const QString &t_tag)
{
    const QColor background = tagColor(t_tag);

    t_painter->save();
    t_painter->setRenderHint(QPainter::Antialiasing);
    t_painter->setPen(Qt::NoPen);
    t_painter->setBrush(background);
    t_painter->drawRoundedRect(t_rect, CornerRadius, CornerRadius);
    t_painter->setPen(tagTextColor(background));
    t_painter->drawText(t_rect, Qt::AlignCenter, t_tag);
    t_painter->restore();
}

TagChip::TagChip(const QString &t_tag, QWidget *parent)
    : QWidget{parent}, m_tag(t_tag)
{
    setToolTip(t_tag);
    setCursor(Qt::OpenHandCursor);
}

QString TagChip::tag() const
{
    return m_tag;
}

QSize TagChip::sizeHint() const
{
    return tagChipSizeHint(fontMetrics(), m_tag);
}

void TagChip::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    paintTagChip(&painter, rect(), m_tag);
}

void TagChip::mousePressEvent(QMouseEvent *t_event)
{
    if(t_event->button() == Qt::LeftButton)
    {
        m_pressed = true;
        m_pressPos = t_event->pos();
    }
    QWidget::mousePressEvent(t_event);
}

void TagChip::mouseMoveEvent(QMouseEvent *t_event)
{
    if(m_pressed && (t_event->pos() - m_pressPos).manhattanLength() >= QApplication::startDragDistance())
    {
        m_pressed = false;

        auto *drag = new QDrag(this);
        drag->setMimeData(encodeTagMime({m_tag}));

        QPixmap pixmap(size());
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        paintTagChip(&painter, rect(), m_tag);
        painter.end();
        drag->setPixmap(pixmap);
        drag->setHotSpot(t_event->pos());

        drag->exec(Qt::CopyAction);
        return;
    }
    QWidget::mouseMoveEvent(t_event);
}

void TagChip::contextMenuEvent(QContextMenuEvent *t_event)
{
    QMenu menu;
    menu.addAction("Remove Tag", this, [this](){
        emit removeRequested(m_tag);
    });
    menu.exec(t_event->globalPos());
}

} // namespace photon
