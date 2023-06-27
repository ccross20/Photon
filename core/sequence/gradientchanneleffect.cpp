#include <QPainter>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>
#include "gradientchanneleffect.h"
#include "channel.h"
#include "gui/color/colorselectorwidget.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "util/utils.h"

namespace photon {


GradientMarkerItem::GradientMarkerItem(const QColor &, GradientEffectEditor *t_editor):m_editor(t_editor)
{
    m_path.moveTo(0,0);
    m_path.lineTo(10,10);
    m_path.lineTo(10,40);
    m_path.lineTo(-10, 40);
    m_path.lineTo(-10,10);
    m_path.closeSubpath();
}

QColor GradientMarkerItem::color() const
{
    return m_color;
}

void GradientMarkerItem::setColor(const QColor &t_color)
{
    m_color = t_color;
}

int GradientMarkerItem::index() const
{
    return m_index;
}

void GradientMarkerItem::setIndex(int t_index)
{
    m_index = t_index;
}

QRectF GradientMarkerItem::boundingRect() const
{
    return QRectF(-10,0,20,40);
}

QPainterPath GradientMarkerItem::shape() const
{
    return m_path;
}

void GradientMarkerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{
    painter->fillPath(m_path, m_color);
}

void GradientMarkerItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_editor->editColor(this);
}

void GradientMarkerItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    m_editor->moveColor(this, event->scenePos());
}

void GradientMarkerItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_editor->endMoveColor(this);
}







GradientEffectEditor::GradientEffectEditor(GradientChannelEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{

    auto t = transform();


    for(auto color : m_effect->colors())
    {
        auto marker = new GradientMarkerItem(color.color,this);
        marker->setIndex(m_markers.length());

        marker->setPos(t.map(QPointF(color.time + m_effect->channel()->startTime(),0)));
        addItem(marker);
        m_markers.append(marker);
    }
}

void GradientEffectEditor::relayout(const QRectF &t_sceneRect)
{
    auto t = transform();
    int index = 0;
    double startTime = m_effect->channel()->startTime();
    for(auto marker : m_markers)
    {
        if(marker->opacity() <= 0.0)
            continue;
        marker->setPos(t.map(QPointF(m_effect->colors()[index++].time + startTime,0)));
    }
}

void GradientEffectEditor::mousePressEvent(QMouseEvent *t_event)
{
    ChannelEffectEditor::mousePressEvent(t_event);

    if(t_event->buttons() != Qt::LeftButton)
        return;

    auto t = transform();
    double time = t.inverted().map(t_event->pos()).x() - m_effect->channel()->startTime();

    if(m_effect->addColor(Qt::red, time))
    {
        auto marker = new GradientMarkerItem(Qt::red,this);
        marker->setIndex(m_markers.length());

        marker->setPos(t.map(QPointF(time + m_effect->channel()->startTime(),0)));
        addItem(marker);
        m_markers.append(marker);

        std::sort(m_markers.begin(), m_markers.end(),[](const GradientMarkerItem *a, const GradientMarkerItem *b){
            return a->pos().x() < b->pos().x();
        });

        int index = 0;
        for(auto marker : m_markers)
            marker->setIndex(index++);
    }

}

void GradientEffectEditor::editColor(photon::GradientMarkerItem *t_item)
{
    m_editIndex = t_item->index();
    m_moveCache = m_effect->colors();

    if(m_colorWidget)
    {
        m_colorWidget->setColor(m_effect->colors()[m_editIndex].color);
        return;
    }
    m_colorWidget = new ColorSelectorWidget(m_effect->colors()[m_editIndex].color);
    m_colorWidget->setMinimumWidth(250);

    connect(m_colorWidget,SIGNAL(selectionChanged(QColor)),this,SLOT(setColor(QColor)));

    addWidget(m_colorWidget, "Color");
}

void GradientEffectEditor::moveColor(photon::GradientMarkerItem *t_item, const QPointF &t_position)
{
    auto t = transform();
    double time = t.inverted().map(t_position).x() - m_effect->channel()->startTime();


    t_item->setPos(t.map(QPointF(time + m_effect->channel()->startTime(),0)));

    m_moveCache[m_editIndex].time = time;

    auto tempCache = m_moveCache;

    if(t_position.y() > 250)
    {
        t_item->setOpacity(0);
        tempCache.remove(m_editIndex);
    }
    else
        t_item->setOpacity(1.0);


    std::sort(tempCache.begin(), tempCache.end(),[](const GradientData &a, const GradientData &b){
        return a.time < b.time;
    });


    m_effect->setColors(tempCache);
}

void GradientEffectEditor::endMoveColor(photon::GradientMarkerItem *t_item)
{
    if(t_item->opacity() <= 0.0)
    {
        m_markers.removeOne(t_item);
        delete t_item;

        std::sort(m_markers.begin(), m_markers.end(),[](const GradientMarkerItem *a, const GradientMarkerItem *b){
            return a->pos().x() < b->pos().x();
        });

        int index = 0;
        for(auto marker : m_markers)
            marker->setIndex(index++);
    }
}


void GradientEffectEditor::setColor(const QColor &t_color)
{
    m_effect->replaceColor(m_editIndex, t_color);
}

GradientChannelEffect::GradientChannelEffect()
{

}


QColor GradientChannelEffect::processColor(QColor value, double time) const
{
    if(m_colors.isEmpty())
        return Qt::black;

    QColor lastColor = m_colors.front().color;
    double lastPosition = m_colors.front().time;



    if(time < lastPosition)
        return lastColor;


    for(auto it = m_colors.begin(); it != m_colors.end(); ++it)
    {
        if(time < (*it).time)
        {

            double f = (time-lastPosition)/((*it).time - lastPosition);

            return  blendColors(lastColor, (*it).color,f);
        }
        lastColor = (*it).color;
        lastPosition = (*it).time;
    }
    return lastColor;

    //return QColor::fromHslF(.5 + (std::sin(2 * M_PI * time * (1.0/5))*(.5)),1.0,.5,1.0);
}

ChannelEffectEditor *GradientChannelEffect::createEditor()
{
    return new GradientEffectEditor(this);
}

const QVector<GradientData> &GradientChannelEffect::colors() const
{
    return m_colors;
}

void GradientChannelEffect::setColors(const QVector<GradientData> &t_colors)
{
    m_colors = t_colors;
    updated();
}

void GradientChannelEffect::replaceColor(int t_index, const QColor &t_color)
{
    m_colors[t_index].color = t_color;
    updated();
}

bool GradientChannelEffect::addColor(const QColor &t_color, double t_time)
{
    if(t_time >= 0 && t_time <= channel()->duration())
    {

        for(auto it = m_colors.cbegin(); it != m_colors.cend(); ++it)
        {
            if(t_time < (*it).time)
            {
                m_colors.insert(it, GradientData{t_color, t_time});
                return true;
            }
        }
        m_colors.push_back(GradientData{t_color, t_time});


        return true;
    }
    updated();
    return false;
}

void GradientChannelEffect::readFromJson(const QJsonObject &t_json)
{
    ChannelEffect::readFromJson(t_json);

    if(!t_json.contains("colors"))
        return;

    m_colors.clear();
    auto colorArray = t_json.value("colors").toArray();

    for(auto color : colorArray)
    {
        auto colorObj = color.toObject();
        m_colors.push_back(GradientData{colorObj.value("color").toString(), colorObj.value("time").toDouble()});
    }
}

void GradientChannelEffect::writeToJson(QJsonObject &t_json) const
{
    ChannelEffect::writeToJson(t_json);

    QJsonArray colorArray;
    for(auto it = m_colors.cbegin(); it != m_colors.cend(); ++it)
    {
        QJsonObject colorObj;
        colorObj.insert("color", (*it).color.name(QColor::HexArgb));
        colorObj.insert("time", (*it).time);
        colorArray.append(colorObj);
    }
    t_json.insert("colors", colorArray);
}

EffectInformation GradientChannelEffect::info()
{
    EffectInformation toReturn([](){return new GradientChannelEffect;});
    toReturn.name = "Gradient";
    toReturn.effectId = "photon.effect.gradient";
    toReturn.categories.append("Generator");

    return toReturn;
}



} // namespace photon
