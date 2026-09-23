#ifndef PHOTON_GRADIENTCHANNELEFFECT_H
#define PHOTON_GRADIENTCHANNELEFFECT_H

#include <QGraphicsItem>
#include <QPainterPath>
#include "sequence/channeleffect.h"

namespace photon {

class GradientChannelEffect;
class GradientEffectEditor;

// A handle for one gradient stop, drawn directly on the curve view's own
// colour-over-time preview band so there's a single gradient on screen
// (rather than a second, separately-scaled strip below it). Its scene x
// tracks the stop's real time through the same transform the preview band
// uses; its y is pinned to a fixed row - the preview fills the whole view's
// height and panning/zooming the (meaningless, for a colour channel) y axis
// shouldn't detach the handles from it.
class PHOTONCORE_EXPORT GradientMarkerItem : public QGraphicsItem
{
public:
    GradientMarkerItem(const QColor &, GradientEffectEditor *);

    QColor color() const;
    void setColor(const QColor &);
    double time() const;
    void setTime(double);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

private:
    GradientEffectEditor *m_editor;
    QPainterPath m_path;
    QColor m_color;
    double m_time = 0.0;
};

struct GradientData
{
    GradientData(QColor value, double time):color(value),time(time){}
    QColor color;
    double time;
};

// double-click the band to add a stop (seeded with whatever colour is already
// showing there); double-click a stop to edit its colour; drag a stop to move
// it; right-click a stop to remove it - the same interaction vocabulary as
// GradientWidget elsewhere in the app, just applied to real clip time instead
// of a normalised 0..1 strip.
class GradientEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    GradientEffectEditor(GradientChannelEffect *);

public slots:
    void editColor(photon::GradientMarkerItem *);
    void moveColor(photon::GradientMarkerItem *, const QPointF &);
    void removeColor(photon::GradientMarkerItem *);

protected:
    void relayout(const QRectF &) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;

private:
    QPointF markerScenePos(double time) const;
    // Keeps m_markers ordered by time (each marker is the single source of
    // truth for its own stop - there's no separate index to fall out of sync).
    void resortMarkers();
    // Pushes m_markers' current state to the effect. Called only once
    // m_markers itself is already fully consistent, since this triggers
    // ChannelEffect::updated() -> Channel::effectModified() ->
    // ChannelEffectEditor::effectUpdated() -> relayout(), synchronously and
    // reentrantly, before this call returns.
    void commitColors();

    QVector<GradientMarkerItem*> m_markers;
    GradientChannelEffect *m_effect;
};




class GradientChannelEffect : public ChannelEffect
{
public:




    GradientChannelEffect();

    float * process(float *value, uint size, double time) const override;
    ChannelEffectEditor *createEditor() override;
    const QVector<GradientData> &colors() const;
    void setColors(const QVector<GradientData> &);
    bool addColor(const QColor &, double);
    void replaceColor(int index, const QColor &);

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

    static EffectInformation info();


private:
    QVector<GradientData> m_colors;
    QGradient m_gradient;
};

} // namespace photon

#endif // PHOTON_GRADIENTCHANNELEFFECT_H
