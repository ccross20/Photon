#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include "easeeffect.h"
#include "sequence/channel.h"
#include "sequence/viewer/stackedparameterwidget.h"

namespace photon {


EaseEffectEditor::EaseEffectEditor(EaseEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    //setMaximumHeight(40);


    QDoubleSpinBox *easeInDurationSpin = new QDoubleSpinBox;
    easeInDurationSpin->setMinimum(.001);
    easeInDurationSpin->setMaximum(9999);
    easeInDurationSpin->setValue(m_effect->easeInDuration());
    connect(easeInDurationSpin, &QDoubleSpinBox::valueChanged, this, &EaseEffectEditor::easeInDurationChanged);

    QDoubleSpinBox *easeOutDurationSpin = new QDoubleSpinBox;
    easeOutDurationSpin->setMinimum(.001);
    easeOutDurationSpin->setMaximum(9999);
    easeOutDurationSpin->setValue(m_effect->easeOutDuration());
    connect(easeOutDurationSpin, &QDoubleSpinBox::valueChanged, this, &EaseEffectEditor::easeOutDurationChanged);

    QStringList easeStrings;
    easeStrings << "Linear";
    easeStrings << "In Quad";
    easeStrings << "Out Quad";
    easeStrings << "In Out Quad";
    easeStrings << "Out In Quad";

    easeStrings << "In Cubic";
    easeStrings << "Out Cubic";
    easeStrings << "In Out Cubic";
    easeStrings << "Out In Cubic";

    easeStrings << "In Quart";
    easeStrings << "Out Quart";
    easeStrings << "In Out Quart";
    easeStrings << "Out In Quart";

    easeStrings << "In Quint";
    easeStrings << "Out Quint";
    easeStrings << "In Out Quint";
    easeStrings << "Out In Quint";

    easeStrings << "In Sine";
    easeStrings << "Out Sine";
    easeStrings << "In Out Sine";
    easeStrings << "Out In Sine";

    easeStrings << "In Expo";
    easeStrings << "Out Expo";
    easeStrings << "In Out Expo";
    easeStrings << "Out In Expo";

    easeStrings << "In Circ";
    easeStrings << "Out Circ";
    easeStrings << "In Out Circ";
    easeStrings << "Out In Circ";

    easeStrings << "In Elastic";
    easeStrings << "Out Elastic";
    easeStrings << "In Out Elastic";
    easeStrings << "Out In Elastic";

    easeStrings << "In Back";
    easeStrings << "Out Back";
    easeStrings << "In Out Back";
    easeStrings << "Out In Back";

    easeStrings << "In Bounce";
    easeStrings << "Out Bounce";
    easeStrings << "In Out Bounce";
    easeStrings << "Out In Bounce";

    QComboBox *easeInCombo = new QComboBox;
    easeInCombo->addItems(easeStrings);
    easeInCombo->setCurrentIndex(m_effect->easeInType());
    connect(easeInCombo, &QComboBox::currentIndexChanged, this, &EaseEffectEditor::easeInChanged);

    QComboBox *easeOutCombo = new QComboBox;
    easeOutCombo->addItems(easeStrings);
    easeOutCombo->setCurrentIndex(m_effect->easeOutType());
    connect(easeOutCombo, &QComboBox::currentIndexChanged, this, &EaseEffectEditor::easeOutChanged);


    StackedParameterWidget *paramWidget = new StackedParameterWidget;
    paramWidget->addWidget(easeInDurationSpin, "Ease In Duration");
    paramWidget->addWidget(easeInCombo, "Ease In");
    paramWidget->addWidget(easeOutDurationSpin, "Ease Out Duration");
    paramWidget->addWidget(easeOutCombo, "Ease Out");

    addWidget(paramWidget, "Ease");


    m_parentItem = new QGraphicsRectItem(0,0,0,0);
    addItem(m_parentItem);



    m_pathItem = new QGraphicsPathItem(m_parentItem);
    m_pathItem->setPen(QPen(Qt::cyan, 2));
    m_pathItem->setBrush(Qt::NoBrush);
    //addItem(m_originHandle);
}

void EaseEffectEditor::easeInDurationChanged(double t_value)
{
    m_effect->setEaseInDuration(t_value);
}

void EaseEffectEditor::easeOutDurationChanged(double t_value)
{
    m_effect->setEaseOutDuration(t_value);
}

void EaseEffectEditor::easeInChanged(int t_ease)
{
    m_effect->setEaseInType(static_cast<QEasingCurve::Type>(t_ease));
}

void EaseEffectEditor::easeOutChanged(int t_ease)
{
    m_effect->setEaseOutType(static_cast<QEasingCurve::Type>(t_ease));
}

void EaseEffectEditor::relayout(const QRectF &t_sceneRect)
{
    /*
    auto t = transform();

    double scaledFreq = m_effect->frequency();
    double startTime = m_effect->channel()->startTime();

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
    */
}

EffectInformation EaseEffect::info()
{
    EffectInformation toReturn([](){return new EaseEffect;});
    toReturn.name = "Ease";
    toReturn.effectId = "photon.effect.ease";
    toReturn.categories.append("Modifier");

    return toReturn;
}

EaseEffect::EaseEffect()
{
    m_easingIn = QEasingCurve(m_easeInType);
    m_easingOut = QEasingCurve(m_easeOutType);
}

void EaseEffect::setEaseInDuration(double t_value)
{
    m_easeInDuration = t_value;
    updated();
}

void EaseEffect::setEaseOutDuration(double t_value)
{
    m_easeOutDuration = t_value;
    updated();
}

void EaseEffect::setEaseInType(QEasingCurve::Type t_value)
{
    m_easeInType = t_value;
    m_easingIn.setType(t_value);
    updated();
}

void EaseEffect::setEaseOutType(QEasingCurve::Type t_value)
{
    m_easeOutType = t_value;
    m_easingOut.setType(t_value);
    updated();
}

float * EaseEffect::process(float *value, uint size, double time) const
{
    if(previousEffect())
    {
        value = previousEffect()->process(value, size, time);
    }

    double duration = channel()->duration();
    if(duration <= 0.0)
        duration = m_easeInDuration + m_easeOutDuration;
    double outStart = duration - m_easeOutDuration;

    for(int i = 0; i < size; ++i)
    {
        if(time > outStart)
        {
            value[i] = m_easingOut.valueForProgress(1 - ((time - outStart) / m_easeOutDuration)) * value[i];
        }

        if(time < m_easeInDuration)
        {
            value[i] = m_easingIn.valueForProgress(time / m_easeInDuration) * value[i];
        }
    }

    return value;
}

ChannelEffectEditor *EaseEffect::createEditor()
{
    return new EaseEffectEditor(this);
}

void EaseEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("ease-in-duration"))
        m_easeInDuration = t_json.value("ease-in-duration").toDouble();
    if(t_json.contains("ease-out-duration"))
        m_easeOutDuration = t_json.value("ease-out-duration").toDouble();
    if(t_json.contains("ease-in-type"))
    {
        m_easeInType = static_cast<QEasingCurve::Type>(t_json.value("ease-in-type").toInt());
        m_easingIn.setType(m_easeInType);
    }
    if(t_json.contains("ease-out-type"))
    {
        m_easeOutType = static_cast<QEasingCurve::Type>(t_json.value("ease-out-type").toInt());
        m_easingOut.setType(m_easeOutType);
    }
}

void EaseEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("ease-in-duration", m_easeInDuration);
    t_json.insert("ease-out-duration", m_easeOutDuration);
    t_json.insert("ease-in-type", m_easeInType);
    t_json.insert("ease-out-type", m_easeOutType);
}

} // namespace photon
