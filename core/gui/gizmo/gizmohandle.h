#ifndef PHOTON_GIZMOHANDLE_H
#define PHOTON_GIZMOHANDLE_H

#include <QGraphicsItem>
#include <functional>
#include "photon-global.h"

namespace photon {

class GizmoGroup;

/**
 * A single draggable handle that lives in *data* space (e.g. time / value) rather
 * than screen pixels. The owning GizmoGroup maps the handle's logical position
 * through the viewer's data->screen transform to place it, and maps a drag back
 * to data space before handing it to the effect. Effects therefore declare a
 * getter (where is this handle?) and a setter (the user moved it here) in their
 * own units, with no manual pixel math.
 *
 * Two styles are supported so the same primitive covers spline editing: an
 * Anchor is a filled square on the curve; a Tangent is a small diamond, meant to
 * be positioned relative to its parent anchor and used as a cubic control point.
 */
class PHOTONCORE_EXPORT GizmoHandle : public QGraphicsItem
{
public:
    enum Style
    {
        Anchor,
        Tangent
    };

    GizmoHandle(GizmoGroup *group, Style style = Anchor);

    // Where the handle sits, in data space. Required.
    void setDataGetter(std::function<QPointF()> getter);
    // Called with the dragged-to data-space position. Omit for a read-only handle.
    void setDataSetter(std::function<void(QPointF)> setter);
    // Which axes the user may drag along (default: both).
    void setConstraint(Qt::Orientations constraint);
    void setSize(qreal size);
    void setColor(const QColor &color);

    Style style() const { return m_style; }
    QPointF dataPosition() const;      // current logical position (from the getter)
    void refreshFromData();            // reposition on screen from the getter

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    GizmoGroup *m_group;
    Style m_style;
    std::function<QPointF()> m_getter;
    std::function<void(QPointF)> m_setter;
    Qt::Orientations m_constraint = Qt::Horizontal | Qt::Vertical;
    qreal m_size = 10;
    QColor m_color = Qt::cyan;
    bool m_applying = false;   // set while we reposition, to ignore our own itemChange
};

} // namespace photon

#endif // PHOTON_GIZMOHANDLE_H
