#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <qmath.h>
#include "stuttereffect.h"
#include "sequence/channel.h"
#include "sequence/clip.h"


namespace photon {

StutterEffectEditor::StutterEffectEditor(StutterEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
/*
    QDoubleSpinBox *durationSpin = new QDoubleSpinBox;
    durationSpin->setValue(m_effect->duration());
    connect(durationSpin, &QDoubleSpinBox::valueChanged, this, &StutterEffectEditor::durationChanged);


    QDoubleSpinBox *gapSpin = new QDoubleSpinBox;
    gapSpin->setValue(m_effect->gap());
    connect(gapSpin, &QDoubleSpinBox::valueChanged, this, &StutterEffectEditor::gapChanged);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(new QLabel("Duration:"));
    hLayout->addWidget(durationSpin);
    hLayout->addWidget(new QLabel("Gap:"));
    hLayout->addWidget(gapSpin);
    hLayout->addStretch();
    setLayout(hLayout);
    */


    m_parentItem = new QGraphicsRectItem(0,0,0,0);
    addItem(m_parentItem);


    m_durationHandle = new RectangleGizmo(QRectF(-5,-5,10,10),[this](QPointF pt){
        m_effect->setDuration(pt.x()/scale().x());
    });
    m_durationHandle->setParentItem(m_parentItem);

    m_gapHandle = new RectangleGizmo(QRectF(-5,-5,10,10),[this](QPointF pt){
        m_effect->setGap(pt.x()/scale().x());
    });
    m_gapHandle->setParentItem(m_durationHandle);
}

void StutterEffectEditor::durationChanged(double t_value)
{
    m_effect->setDuration(t_value);
}

void StutterEffectEditor::gapChanged(double t_value)
{
    m_effect->setGap(t_value);
}

void StutterEffectEditor::relayout(const QRectF &t_sceneRect)
{
    auto t = transform();

    double gap = m_effect->gap();
    double duration = m_effect->duration();
    double startTime = m_effect->channel()->clip()->startTime();

    double x = startTime;

    if(t_sceneRect.left() > startTime)
    {
        x = (ceil((t_sceneRect.left() - startTime) / gap) * gap) + startTime;
    }
    m_parentItem->setPos(t.map(QPointF(x,0)));

    m_gapHandle->setPos(QPointF(gap * scale().x(),0));
    m_durationHandle->setPos(QPointF(duration * scale().x(),0));

    /*
    QPainterPath path;
    path.moveTo(m_frequencyHandle->pos());
    path.lineTo(m_originHandle->pos());
    path.lineTo(m_amplitudeHandle->pos());
    m_pathItem->setPath(path);
    */
}



EffectInformation StutterEffect::info()
{
    EffectInformation toReturn([](){return new StutterEffect;});
    toReturn.name = "Stutter";
    toReturn.effectId = "photon.effect.stutter";
    toReturn.categories.append("Time");

    return toReturn;
}



StutterEffect::StutterEffect() : ChannelEffect()
{

}

void StutterEffect::setDuration(double t_value)
{
    m_duration = t_value;
    updated();

}

void StutterEffect::setGap(double t_value)
{
    m_gap = t_value;
    updated();
}

double StutterEffect::process(double value, double time) const
{

    if(previousEffect())
    {

        double period = m_duration + m_gap;
        double i = std::floor(time / period);
        double remainder = time -  (period * i);

        double offset = i * m_duration;
        //double offset = i * period;

        if(remainder < m_duration)
            offset += remainder;
        else
            offset += m_duration;

        return previousEffect()->process(value, offset);
    }


    return value;
}

ChannelEffectEditor *StutterEffect::createEditor()
{
    return new StutterEffectEditor(this);
}

void StutterEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);
    if(t_json.contains("duration"))
        m_duration = t_json.value("duration").toDouble();
    if(t_json.contains("gap"))
        m_gap = t_json.value("gap").toDouble();
}

void StutterEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);
    t_json.insert("duration", m_duration);
    t_json.insert("gap", m_gap);
}

} // namespace photon
