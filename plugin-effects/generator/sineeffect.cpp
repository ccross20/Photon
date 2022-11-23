#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include "sineeffect.h"
#include "sequence/channel.h"
#include "sequence/clip.h"

namespace photon {



SquareHandle::SquareHandle(SineEffect *t_effect, std::function<void(QPointF)> t_callback):QGraphicsItem(),m_effect(t_effect),m_callback(t_callback)
{
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsScenePositionChanges);
    setAcceptHoverEvents(true);
}

void SquareHandle::setOrientation(Qt::Orientation t_orientation)
{
    m_orientation = t_orientation;
}

QRectF SquareHandle::boundingRect() const
{
    return QRectF(-5,-5,10,10);
}

QPainterPath SquareHandle::shape() const
{
    QPainterPath path;
    path.addRect(-5,-5,10,10);
    return path;
}

void SquareHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);

    m_callback(pos());
}

void SquareHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{
    QColor fillColor = Qt::cyan;
    if (option->state & QStyle::State_MouseOver)
        fillColor = Qt::black;


    painter->fillRect(-5,-5,10,10, fillColor);
}

QVariant SquareHandle::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{

    if (change == ItemPositionChange && scene()) {
        // value is the new position.

        QPointF newPos = value.toPointF();
        if(m_orientation == Qt::Horizontal)
            newPos.setY(pos().y());
        else if(m_orientation == Qt::Vertical)
            newPos.setX(pos().x());
        return newPos;
    }

    return QGraphicsItem::itemChange(change, value);
}




SineEffectEditor::SineEffectEditor(SineEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    //setMaximumHeight(40);

    QDoubleSpinBox *freqSpin = new QDoubleSpinBox;
    freqSpin->setValue(m_effect->frequency());
    connect(freqSpin, &QDoubleSpinBox::valueChanged, this, &SineEffectEditor::frequencyChanged);


    QDoubleSpinBox *ampSpin = new QDoubleSpinBox;
    ampSpin->setValue(m_effect->amplitude());
    connect(ampSpin, &QDoubleSpinBox::valueChanged, this, &SineEffectEditor::amplitudeChanged);

    m_parentItem = new QGraphicsRectItem(0,0,0,0);
    addItem(m_parentItem);

    m_frequencyHandle = new SquareHandle(t_effect,[this](QPointF pt){
        m_effect->setFrequency(pt.x()/scale().x());
    });
    m_frequencyHandle->setParentItem(m_parentItem);


    m_amplitudeHandle = new SquareHandle(t_effect,[this](QPointF pt){
        m_effect->setAmplitude(pt.y()/scale().y());
    });
    m_amplitudeHandle->setOrientation(Qt::Vertical);
    m_amplitudeHandle->setParentItem(m_parentItem);

    m_originHandle = new SquareHandle(t_effect,[](QPointF pt){

    });
    m_originHandle->setParentItem(m_parentItem);

    m_pathItem = new QGraphicsPathItem(m_parentItem);
    m_pathItem->setPen(QPen(Qt::cyan, 2));
    m_pathItem->setBrush(Qt::NoBrush);
    //addItem(m_originHandle);
}

void SineEffectEditor::frequencyChanged(double t_value)
{
    m_effect->setFrequency(t_value);
}

void SineEffectEditor::amplitudeChanged(double t_value)
{
    m_effect->setAmplitude(t_value);
}

void SineEffectEditor::relayout(const QRectF &t_sceneRect)
{
    auto t = transform();

    double scaledFreq = m_effect->frequency();
    double startTime = m_effect->channel()->clip()->startTime();

    double x = startTime;

    if(t_sceneRect.left() > startTime)
    {
        x = (ceil((t_sceneRect.left() - startTime) / scaledFreq) * scaledFreq) + startTime;
    }

    m_referencePt = QPoint(x,0);

    m_parentItem->setPos(t.map(QPointF(x,0)));

    m_originHandle->setPos(QPointF(0,0));
    m_frequencyHandle->setPos(QPointF(scaledFreq * scale().x(),0));
    m_amplitudeHandle->setPos(QPointF(0, m_effect->amplitude() * scale().y()));

    QPainterPath path;
    path.moveTo(m_frequencyHandle->pos());
    path.lineTo(m_originHandle->pos());
    path.lineTo(m_amplitudeHandle->pos());
    m_pathItem->setPath(path);
}



EffectInformation SineEffect::info()
{
    EffectInformation toReturn([](){return new SineEffect;});
    toReturn.name = "Sinewave";
    toReturn.effectId = "photon.effect.sinewave";
    toReturn.categories.append("Generator");

    return toReturn;
}



SineEffect::SineEffect() : ChannelEffect()
{

}

void SineEffect::setFrequency(double t_value)
{
    m_frequency = t_value;
    updated();

}

void SineEffect::setAmplitude(double t_value)
{
    m_amplitude = t_value;
    updated();
}

double SineEffect::process(double value, double time) const
{
    if(previousEffect())
    {
        value = previousEffect()->process(value, time);
    }
    return value + (std::sin(2 * M_PI * time * (1.0/m_frequency))*(m_amplitude));
}

ChannelEffectEditor *SineEffect::createEditor()
{
    return new SineEffectEditor(this);
}

void SineEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("amplitude"))
        m_amplitude = t_json.value("amplitude").toDouble();
    if(t_json.contains("frequency"))
        m_frequency = t_json.value("frequency").toDouble();
}

void SineEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("amplitude", m_amplitude);
    t_json.insert("frequency", m_frequency);
}

} // namespace photon
