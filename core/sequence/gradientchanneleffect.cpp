#include <QPainter>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>
#include "gradientchanneleffect.h"
#include "channel.h"
#include "color/colorselectordialog.h"
#include "channel/parameter/colorchannelparameter.h"
#include "util/utils.h"

namespace photon {

static const qreal kMarkerY = 4;

GradientMarkerItem::GradientMarkerItem(const QColor &t_color, GradientEffectEditor *t_editor):m_editor(t_editor),m_color(t_color)
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

double GradientMarkerItem::time() const
{
    return m_time;
}

void GradientMarkerItem::setTime(double t_time)
{
    m_time = t_time;
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
    // An outline so a stop's handle stays visible even when its colour is
    // close to the preview band behind it (e.g. a black or very dark stop).
    painter->setPen(QPen(Qt::white, 1));
    painter->drawPath(m_path);
}

void GradientMarkerItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        m_editor->removeColor(this);
        return;
    }
    // Left button: accept so the drag is delivered to mouseMoveEvent(); colour
    // editing is a double-click (see mouseDoubleClickEvent), matching
    // GradientWidget's convention elsewhere in the app.
}

void GradientMarkerItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    m_editor->moveColor(this, event->scenePos());
}

void GradientMarkerItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    m_editor->editColor(this);
}


GradientEffectEditor::GradientEffectEditor(GradientChannelEffect *t_effect):ChannelEffectEditor(t_effect),m_effect(t_effect)
{
    for(auto color : m_effect->colors())
    {
        auto marker = new GradientMarkerItem(color.color, this);
        marker->setTime(color.time);
        marker->setPos(markerScenePos(color.time));
        addItem(marker);
        m_markers.append(marker);
    }
}

QPointF GradientEffectEditor::markerScenePos(double t_time) const
{
    auto t = transform();
    double startTime = m_effect->channel()->startTime();
    double x = t.map(QPointF(t_time + startTime, 0)).x();
    return QPointF(x, kMarkerY);
}

void GradientEffectEditor::relayout(const QRectF &)
{
    // Reads each marker's own time rather than cross-referencing
    // m_effect->colors() by index - this runs reentrantly from inside
    // commitColors() below (setColors() -> updated() -> effectModified() ->
    // effectUpdated() -> remakeTransform() -> this), while m_markers and the
    // effect's colour list can briefly disagree on size/order mid-edit.
    for(auto marker : m_markers)
        marker->setPos(markerScenePos(marker->time()));
}

void GradientEffectEditor::resortMarkers()
{
    std::sort(m_markers.begin(), m_markers.end(),[](const GradientMarkerItem *a, const GradientMarkerItem *b){
        return a->time() < b->time();
    });
}

void GradientEffectEditor::commitColors()
{
    QVector<GradientData> colors;
    colors.reserve(m_markers.size());
    for(auto marker : m_markers)
        colors.append(GradientData{marker->color(), marker->time()});

    m_effect->setColors(colors);
}

void GradientEffectEditor::mouseDoubleClickEvent(QMouseEvent *t_event)
{
    if(t_event->button() != Qt::LeftButton)
        return;

    auto t = transform();
    double time = t.inverted().map(t_event->pos()).x() - m_effect->channel()->startTime();
    time = qBound(0.0, time, m_effect->channel()->duration());

    // Seed the new stop with whatever colour is already showing at this time,
    // matching GradientWidget's own add-stop behaviour, rather than a fixed
    // colour that always needs to be changed right after adding.
    float values[4];
    ColorChannelParameter::colorToChannels(m_effect->channel()->info().defaultValue.value<QColor>(), values);
    QColor seedColor = ColorChannelParameter::channelsToColor(m_effect->process(values, 4, time));

    auto marker = new GradientMarkerItem(seedColor, this);
    marker->setTime(time);
    marker->setPos(markerScenePos(time));
    addItem(marker);
    m_markers.append(marker);
    resortMarkers();

    commitColors();
}

void GradientEffectEditor::editColor(photon::GradientMarkerItem *t_item)
{
    auto *dialog = new ColorSelectorDialog(t_item->color(), window());
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
    dialog->raise();
    dialog->activateWindow();

    connect(dialog, &ColorSelectorDialog::selectionChanged, this, [this, t_item](QColor color){
        if(!m_markers.contains(t_item))
            return;
        t_item->setColor(color);
        t_item->update();
        commitColors();
    });
}

void GradientEffectEditor::moveColor(photon::GradientMarkerItem *t_item, const QPointF &t_position)
{
    auto t = transform();
    double time = t.inverted().map(t_position).x() - m_effect->channel()->startTime();
    time = qBound(0.0, time, m_effect->channel()->duration());

    t_item->setTime(time);
    t_item->setPos(markerScenePos(time));

    // Reorder BEFORE committing - resortMarkers() only touches m_markers
    // (never m_effect), so it's always safe to run ahead of the reentrant
    // relayout() that commitColors() triggers.
    resortMarkers();
    commitColors();
}

void GradientEffectEditor::removeColor(photon::GradientMarkerItem *t_item)
{
    // Keep at least two stops so the gradient stays meaningful, matching
    // GradientWidget's own floor.
    if(m_markers.size() <= 2)
        return;

    m_markers.removeOne(t_item);
    delete t_item;

    commitColors();
}

GradientChannelEffect::GradientChannelEffect()
{

}

float * GradientChannelEffect::process(float *value, uint size, double time) const
{
    if(m_colors.isEmpty())
        return colorToValues(Qt::black, value, size);

    QColor lastColor = m_colors.front().color;
    double lastPosition = m_colors.front().time;



    if(time < lastPosition)
        return colorToValues(lastColor, value, size);


    for(auto it = m_colors.begin(); it != m_colors.end(); ++it)
    {
        if(time < (*it).time)
        {

            double f = (time-lastPosition)/((*it).time - lastPosition);

            return colorToValues(blendColors(lastColor, (*it).color,f), value, size);
        }
        lastColor = (*it).color;
        lastPosition = (*it).time;
    }
    return colorToValues(lastColor, value, size);

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
                updated();
                return true;
            }
        }
        m_colors.push_back(GradientData{t_color, t_time});
        updated();

        return true;
    }
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
