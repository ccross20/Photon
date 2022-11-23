#ifndef PHOTON_SINEEFFECT_H
#define PHOTON_SINEEFFECT_H

#include <QGraphicsItem>
#include "sequence/channeleffect.h"

namespace photon {

class SineEffect;

class SquareHandle : public QGraphicsItem
{

public:
    SquareHandle(SineEffect *, std::function<void(QPointF)>);
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void setOrientation(Qt::Orientation);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;
    QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) override;


protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

private:
    SineEffect *m_effect;
    Qt::Orientation m_orientation = Qt::Horizontal;
    std::function<void(QPointF)> m_callback;
};





class SineEffectEditor : public ChannelEffectEditor
{
    Q_OBJECT
public:
    SineEffectEditor(SineEffect *);

private slots:
    void frequencyChanged(double);
    void amplitudeChanged(double);

protected:
    void relayout(const QRectF &) override;

private:
    SineEffect *m_effect;
    QPointF m_referencePt;
    QGraphicsRectItem *m_parentItem;
    SquareHandle *m_originHandle;
    SquareHandle *m_frequencyHandle;
    SquareHandle *m_amplitudeHandle;
    QGraphicsPathItem *m_pathItem;

};


class SineEffect : public ChannelEffect
{
public:
    SineEffect();

    void setFrequency(double);
    void setAmplitude(double);
    double frequency() const{return m_frequency;}
    double amplitude() const{return m_amplitude;}
    double process(double value, double time) const override;
    ChannelEffectEditor *createEditor() override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;


    static EffectInformation info();


private:
    double m_amplitude = 1.0;
    double m_frequency = 10.0;
};

} // namespace photon

#endif // PHOTON_SINEEFFECT_H
