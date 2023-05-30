#ifndef PHOTON_GRADIENTCHANNELEFFECT_H
#define PHOTON_GRADIENTCHANNELEFFECT_H

#include "sequence/channeleffect.h"
#include "gui/gizmo/rectanglegizmo.h"

namespace photon {

class GradientChannelEffect;
class GradientEffectEditor;



class PHOTONCORE_EXPORT GradientMarkerItem : public QGraphicsItem
{
public:
    GradientMarkerItem(const QColor &, GradientEffectEditor *);

    QColor color() const;
    void setColor(const QColor &);
    int index() const;
    void setIndex(int);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    GradientEffectEditor *m_editor;
    QPainterPath m_path;
    QColor m_color;
    int m_index;
};

struct GradientData
{
    GradientData(QColor value, double time):color(value),time(time){}
    QColor color;
    double time;
};

class ColorSelectorWidget;

class GradientEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    GradientEffectEditor(GradientChannelEffect *);

public slots:
    void editColor(photon::GradientMarkerItem *);
    void moveColor(photon::GradientMarkerItem *, const QPointF &);
    void endMoveColor(photon::GradientMarkerItem *);
    void setColor(const QColor &color);

protected:
    void relayout(const QRectF &) override;
    void mousePressEvent(QMouseEvent *) override;

private:
    ColorSelectorWidget *m_colorWidget = nullptr;
    QVector<GradientData> m_moveCache;
    GradientMarkerItem *m_activeMarker = nullptr;
    GradientChannelEffect *m_effect;
    QVector<GradientMarkerItem*> m_markers;
    int m_editIndex = -1;

};




class GradientChannelEffect : public ChannelEffect
{
public:




    GradientChannelEffect();

    QColor processColor(QColor value, double time) const override;
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
