#include <QPen>
#include <QDoubleSpinBox>
#include "constantchanneleffect.h"
#include "sequence/viewer/stackedparameterwidget.h"

namespace photon {

ConstantEffectEditor::ConstantEffectEditor(ConstantChannelEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    //setMaximumHeight(40);


    QDoubleSpinBox *constantSpin = new QDoubleSpinBox;
    constantSpin->setMinimum(-10000);
    constantSpin->setMaximum(10000);
    constantSpin->setValue(m_effect->value());
    connect(constantSpin, &QDoubleSpinBox::valueChanged, this, &ConstantEffectEditor::valueChanged);

    QDoubleSpinBox *rateSpin = new QDoubleSpinBox;
    rateSpin->setMinimum(-10000);
    rateSpin->setMaximum(10000);
    rateSpin->setValue(m_effect->rate()*100);
    connect(rateSpin, &QDoubleSpinBox::valueChanged, this, &ConstantEffectEditor::rateChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(constantSpin, "Value");
    paramWidget->addWidget(rateSpin, "Rate");

    addWidget(paramWidget, "Constant");

    m_parentItem = new QGraphicsRectItem(0,0,0,0);
    addItem(m_parentItem);


    m_originHandle = new RectangleGizmo(QRectF(-5,-5,10,10),[this, constantSpin](QPointF pt){


            m_effect->setValue(transform().inverted().map(pt).y());
        constantSpin->setValue(m_effect->value());
        }, RectangleGizmo::PositionAbsolute);
    m_originHandle->setParentItem(m_parentItem);
    //m_originHandle->setOrientation(Qt::Vertical | Qt::Horizontal);
    addItem(m_originHandle);

    m_rateHandle = new RectangleGizmo(QRectF(-5,-5,10,10),[this, rateSpin](QPointF pt){
        m_effect->setRate( ((pt.y())/-scale().y())/5.0);
        rateSpin->setValue(m_effect->rate()*100.0);
    });
    m_rateHandle->setParentItem(m_parentItem);

    m_pathItem = new QGraphicsPathItem();
    m_pathItem->setPen(QPen(Qt::cyan, 2));
    m_pathItem->setBrush(Qt::NoBrush);
    addItem(m_pathItem);
}

void ConstantEffectEditor::valueChanged(double t_value)
{
    m_effect->setValue(t_value);
}

void ConstantEffectEditor::rateChanged(double t_value)
{
    m_effect->setRate(t_value/100.0);
}

void ConstantEffectEditor::relayout(const QRectF &t_sceneRect)
{
    auto t = transform();

    double y = m_effect->value();


    m_parentItem->setPos(t.map(QPointF(t_sceneRect.center().x(),y)));
    m_originHandle->setPos(QPointF(0,0));

    QPainterPath path;
    path.moveTo(t.map(QPointF(t_sceneRect.left(), y)));
    path.lineTo(t.map(QPointF(t_sceneRect.right(), y)));
    m_pathItem->setPath(path);


    m_rateHandle->setPos(QPointF{5* scale().x(),m_effect->rate()*5.0 * -scale().y()});

    path.moveTo(t.map(m_originHandle->pos()));
    path.lineTo(t.map(m_rateHandle->pos()));
}



EffectInformation ConstantChannelEffect::info()
{
    EffectInformation toReturn([](){return new ConstantChannelEffect;});
    toReturn.name = "Constant";
    toReturn.effectId = "photon.effect.constant";
    toReturn.categories.append("Generator");

    return toReturn;
}



ConstantChannelEffect::ConstantChannelEffect() : ChannelEffect()
{

}

void ConstantChannelEffect::setValue(double t_value)
{
    m_value = t_value;
    updated();
}

void ConstantChannelEffect::setRate(double t_value)
{
    m_rate = t_value;
    updated();
}

QVariant ConstantChannelEffect::process(QVariant value, double time) const
{

    return m_value + (m_rate * time);
}

ChannelEffectEditor *ConstantChannelEffect::createEditor()
{
    return new ConstantEffectEditor(this);
}

void ConstantChannelEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    m_value = t_json.value("value").toDouble();
    m_rate = t_json.value("rate").toDouble();
}

void ConstantChannelEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("value", m_value);
    t_json.insert("rate", m_rate);
}

} // namespace photon
