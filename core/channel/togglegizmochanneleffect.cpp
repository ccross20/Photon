#include <QPen>
#include <QDoubleSpinBox>
#include "ToggleGizmoChannelEffect.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "sequence/channel.h"
#include "surface/surfacegizmocontainer.h"
#include "surface/togglegizmo.h"

namespace photon {

ToggleGizmoChannelEffectEditor::ToggleGizmoChannelEffectEditor(ToggleGizmoChannelEffect *t_effect):GizmoChannelEffectEditor(t_effect),m_effect(t_effect)
{
    //setMaximumHeight(40);


    QDoubleSpinBox *onValueSpin = new QDoubleSpinBox;
    onValueSpin->setMinimum(-10000);
    onValueSpin->setMaximum(10000);
    onValueSpin->setValue(m_effect->onValue());
    connect(onValueSpin, &QDoubleSpinBox::valueChanged, this, &ToggleGizmoChannelEffectEditor::onValueChanged);

    QDoubleSpinBox *offValueSpin = new QDoubleSpinBox;
    offValueSpin->setMinimum(-10000);
    offValueSpin->setMaximum(10000);
    offValueSpin->setValue(m_effect->offValue());
    connect(offValueSpin, &QDoubleSpinBox::valueChanged, this, &ToggleGizmoChannelEffectEditor::offValueChanged);

    StackedParameterWidget *paramWidget = createParameterWidget(t_effect);
    paramWidget->addWidget(onValueSpin, "On Value");
    paramWidget->addWidget(offValueSpin, "Off Value");

    addWidget(paramWidget, "Toggle Gizmo");

    m_parentItem = new QGraphicsRectItem(0,0,0,0);
    addItem(m_parentItem);


    m_originHandle = new RectangleGizmo(QRectF(-5,-5,10,10),[this, onValueSpin](QPointF pt){


        m_effect->setOnValue(transform().inverted().map(pt).y());
        onValueSpin->setValue(m_effect->onValue());
    }, RectangleGizmo::PositionAbsolute);
    m_originHandle->setParentItem(m_parentItem);
    //m_originHandle->setOrientation(Qt::Vertical | Qt::Horizontal);
    addItem(m_originHandle);

    m_rateHandle = new RectangleGizmo(QRectF(-5,-5,10,10),[this, offValueSpin](QPointF pt){
        m_effect->setOffValue(transform().inverted().map(pt).y());
        offValueSpin->setValue(m_effect->onValue());
    });
    m_rateHandle->setParentItem(m_parentItem);

    m_pathItem = new QGraphicsPathItem();
    m_pathItem->setPen(QPen(Qt::cyan, 2));
    m_pathItem->setBrush(Qt::NoBrush);
    addItem(m_pathItem);
}

void ToggleGizmoChannelEffectEditor::onValueChanged(double t_value)
{
    m_effect->setOnValue(t_value);
}

void ToggleGizmoChannelEffectEditor::offValueChanged(double t_value)
{
    m_effect->setOffValue(t_value);
}

void ToggleGizmoChannelEffectEditor::relayout(const QRectF &t_sceneRect)
{
    auto t = transform();

    double y = m_effect->onValue();


    m_parentItem->setPos(t.map(QPointF(t_sceneRect.center().x(),y)));
    m_originHandle->setPos(QPointF(0,0));

    QPainterPath path;
    path.moveTo(t.map(QPointF(t_sceneRect.left(), y)));
    path.lineTo(t.map(QPointF(t_sceneRect.right(), y)));
    m_pathItem->setPath(path);


    m_rateHandle->setPos(QPointF{0, m_effect->offValue()});

    path.moveTo(t.map(m_originHandle->pos()));
    path.lineTo(t.map(m_rateHandle->pos()));
}



EffectInformation ToggleGizmoChannelEffect::info()
{
    EffectInformation toReturn([](){return new ToggleGizmoChannelEffect;});
    toReturn.name = "Toggle Gizmo";
    toReturn.effectId = "photon.effect.toggle-gizmo";
    toReturn.categories.append("Gizmo");

    return toReturn;
}



ToggleGizmoChannelEffect::ToggleGizmoChannelEffect() : GizmoChannelEffect(ToggleGizmo::GizmoId)
{

}

void ToggleGizmoChannelEffect::setOnValue(double t_value)
{
    m_onValue = t_value;
    updated();
}

void ToggleGizmoChannelEffect::setOffValue(double t_value)
{
    m_offValue = t_value;
    updated();
}

float * ToggleGizmoChannelEffect::process(float *value, uint size, double time) const
{

    bool isPressed = false;
    if(gizmo())
    {
        ToggleGizmo *toggleGizmo = dynamic_cast<ToggleGizmo*>(gizmo());
        //qDebug() <<  toggleGizmo->isPressed();
        isPressed = toggleGizmo->isPressed();
    }

    for(int i = 0; i < size; ++i)
    {
        value[i] = isPressed ? m_onValue : m_offValue;
    }
    return value;
}

ChannelEffectEditor *ToggleGizmoChannelEffect::createEditor()
{
    return new ToggleGizmoChannelEffectEditor(this);
}

void ToggleGizmoChannelEffect::readFromJson(const QJsonObject &t_json)
{
    GizmoChannelEffect::readFromJson(t_json);
    m_onValue = t_json.value("onValue").toDouble();
    m_offValue = t_json.value("offValue").toDouble();
}

void ToggleGizmoChannelEffect::writeToJson(QJsonObject &t_json) const
{
    GizmoChannelEffect::writeToJson(t_json);
    t_json.insert("onValue", m_onValue);
    t_json.insert("offValue", m_offValue);
}

} // namespace photon

