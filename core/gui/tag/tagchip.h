#ifndef PHOTON_TAGCHIP_H
#define PHOTON_TAGCHIP_H

#include <QWidget>
#include "photon-global.h"

class QPainter;
class QFontMetrics;

namespace photon {

// Chip geometry shared between TagChip's own paintEvent() and the Project
// panel tree's tag delegate, so a chip painted as a real widget and a chip
// painted directly into a tree cell can never visually drift apart.
PHOTONCORE_EXPORT QSize tagChipSizeHint(const QFontMetrics &metrics, const QString &tag);
PHOTONCORE_EXPORT void paintTagChip(QPainter *painter, const QRect &rect, const QString &tag);

// One colored, named tag chip. Drag source (press-and-drag starts a QDrag
// carrying the tag's name) and delete affordance (right-click -> Remove Tag)
// in one small widget; TagEditorWidget lays a row of these out in a
// FlowLayout.
class PHOTONCORE_EXPORT TagChip : public QWidget
{
    Q_OBJECT
public:
    explicit TagChip(const QString &tag, QWidget *parent = nullptr);

    QString tag() const;

    QSize sizeHint() const override;

signals:
    void removeRequested(QString tag);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QString m_tag;
    QPoint m_pressPos;
    bool m_pressed = false;
};

} // namespace photon

#endif // PHOTON_TAGCHIP_H
