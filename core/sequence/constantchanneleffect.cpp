#include <QPen>
#include <QDoubleSpinBox>
#include "constantchanneleffect.h"
#include "sequence/viewer/stackedparameterwidget.h"

namespace photon {

ConstantEffectEditor::ConstantEffectEditor(ConstantChannelEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    //setMaximumHeight(40);


    QDoubleSpinBox *constantSpin = new QDoubleSpinBox;
    constantSpin->setMinimum(-255);
    constantSpin->setMaximum(255);
    constantSpin->setValue(m_effect->value());
    connect(constantSpin, &QDoubleSpinBox::valueChanged, this, &ConstantEffectEditor::valueChanged);

    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(constantSpin, "Value");

    addWidget(paramWidget, "Constant");


    m_originHandle = new RectangleGizmo(QRectF(-5,-5,10,10),[this, constantSpin](QPointF pt){
        m_effect->setValue( (pt.y() - offset().y())/-scale().y());
        constantSpin->setValue(m_effect->value());
    });
    //m_originHandle->setOrientation(Qt::Vertical | Qt::Horizontal);
    addItem(m_originHandle);


    m_pathItem = new QGraphicsPathItem();
    m_pathItem->setPen(QPen(Qt::cyan, 2));
    m_pathItem->setBrush(Qt::NoBrush);
    addItem(m_pathItem);
}

void ConstantEffectEditor::valueChanged(double t_value)
{
    m_effect->setValue(t_value);
}

void ConstantEffectEditor::relayout(const QRectF &t_sceneRect)
{
    auto t = transform();

    double y = m_effect->value();


    m_originHandle->setPos(t.map(QPointF(t_sceneRect.center().x(),y)));

    QPainterPath path;
    path.moveTo(t.map(QPointF(t_sceneRect.left(), y)));
    path.lineTo(t.map(QPointF(t_sceneRect.right(), y)));
    m_pathItem->setPath(path);
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

double ConstantChannelEffect::process(double value, double time) const
{

    return m_value;
}

ChannelEffectEditor *ConstantChannelEffect::createEditor()
{
    return new ConstantEffectEditor(this);
}

void ConstantChannelEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("value"))
        m_value = t_json.value("value").toDouble();
}

void ConstantChannelEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("value", m_value);
}

} // namespace photon
