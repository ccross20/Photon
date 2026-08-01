#include <cmath>
#include <algorithm>
#include <limits>
#include <vector>
#include <QPainter>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QGraphicsLinearLayout>
#include "channeleffecteditor_p.h"
#include "sequence/channeleffect.h"
#include "sequence/channel.h"
#include "sequence/clip.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "channel/parameter/colorchannelparameter.h"
#include "project/project.h"
#include "photoncore.h"

namespace photon {



WidgetContainer::WidgetContainer(QWidget *t_widget, const QString &t_title):QGraphicsWidget(),m_widget(t_widget),m_title(t_title)
{

    //setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    m_widget->setStyleSheet("QLabel{color:white;}");
    setMinimumWidth(m_widget->minimumWidth());

    auto stacked = dynamic_cast<StackedParameterWidget*>(t_widget);
    if(stacked)
        connect(stacked, &StackedParameterWidget::widgetResized, this, &WidgetContainer::sizeUpdated);
}

QRectF WidgetContainer::boundingRect() const
{
    return QRect{0,0,m_widget->width() + (m_inset * 2), m_widget->height() + 25 + (m_inset * 2)};
}

void WidgetContainer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing);

    QColor color = Qt::black;
    color.setAlphaF(.7);

    painter->setBrush(color);

    painter->drawRoundedRect(boundingRect(),8,8, Qt::SizeMode::RelativeSize);

    QFont font;
    font.setBold(true);
    painter->setFont(font);

    painter->setPen(Qt::white);
    //painter->setBrush(Qt::white);
    painter->drawText(QRect(m_inset,m_inset,m_widget->width(),25), Qt::AlignCenter, m_title);
}

void WidgetContainer::addedToScene(QGraphicsScene *t_scene)
{
    QGraphicsLinearLayout *linearLayout = new QGraphicsLinearLayout(Qt::Vertical);
    m_proxy = t_scene->addWidget(m_widget);

    linearLayout->addItem(m_proxy);

    linearLayout->setContentsMargins(m_inset,25 + m_inset, m_inset, m_inset);
    setLayout(linearLayout);
}

void WidgetContainer::sizeUpdated(QSize t_size)
{
    prepareGeometryChange();
}






EffectEditorViewer::EffectEditorViewer(ChannelEffect *t_effect):QGraphicsView(),m_effect(t_effect)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setInteractive(true);
    //setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    m_yOffset = m_effect->viewState().yOffset;
    m_yScale = m_effect->viewState().yScale;


}


void EffectEditorViewer::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);

    if(m_effect->channel()->type()  == ChannelInfo::ChannelTypeColor)
    {
        drawBackgroundColor(painter, rect);
    }
    else
    {
        drawBackgroundNumber(painter, rect);
    }

}

void EffectEditorViewer::drawBackgroundColor(QPainter *painter, const QRectF &rect)
{
    painter->fillRect(rect, QColor(50,50,50));

    /*
    if(m_pathsDirty)
        rebuildColors();

    m_colorsDirty = false;
*/
    painter->resetTransform();

    double startTime = m_effect->channel()->startTime();
    double endTime = m_effect->channel()->endTime();

    double startX = m_transform.map(QPointF(std::max(startTime, m_sceneBounds.left()),0.0)).x();
    double endX = m_transform.map(QPointF(std::min(endTime, m_sceneBounds.right()),0.0)).x();
    auto invTrans = m_transform.inverted();

    QColor initialValue = m_effect->channel()->info().defaultValue.value<QColor>();

    float *values = new float[4];

    for(int x = startX; x < endX; ++x)
    {
        ColorChannelParameter::colorToChannels(initialValue, values);
        values = m_effect->process(values, 4, invTrans.map(QPointF(x,0)).x() - startTime);

        auto c = ColorChannelParameter::channelsToColor(values);

        painter->fillRect(x,0,1,50, c);
    }


    delete [] values;
/*
    int x = 0;
    for(auto it = m_colors.cbegin(); it != m_colors.cend(); ++it)
    {
        painter->fillRect(x++,0,1,50, *it);
        if(x > m_xScale)
            return;
    }

*/
}

void EffectEditorViewer::drawBackgroundNumber(QPainter *painter, const QRectF &rect)
{
    painter->fillRect(rect, QColor(50,50,50));
    if(m_pathsDirty)
        rebuildPaths();

    painter->setPen(QPen(QColor(80,80,80), 0));


    double yScale = m_yScale;


    double rulerUnit = 1;

    double markSpacing =  yScale;

    double inverseSpacing = (1.0/markSpacing)*rulerUnit;
    double zoomMultiple = 1.0;
    int counter = 0;

    if(yScale > 25)
    {
        while(markSpacing * zoomMultiple > 500 && counter < 100)
        {
            zoomMultiple /= 10;
            counter++;
        }
    }
    else{
        while(markSpacing * zoomMultiple < 50 && counter < 100)
        {
            zoomMultiple *= 10;
            counter++;
        }
    }

    //if(yScale > 2)
    {
        markSpacing *= zoomMultiple;
        double unitDivisor = 1.0 / zoomMultiple;

        double markStartPos = std::remainder(m_yOffset, markSpacing) - markSpacing;

        //qDebug() << markStartPos << "  " << markSpacing;
        double maxSize = height() ;

        double markSubSpacing = markSpacing * .1;
        double markPos = markStartPos;

        if(markSubSpacing > 4)
        {
            while(markPos < maxSize && markSubSpacing > 4)
            {
                painter->drawLine(0, static_cast<int>(markPos),width(), static_cast<int>(markPos));
                markPos += markSubSpacing;
            }

            painter->setPen(QPen(QColor(130,130,130), 0));
            markPos = markStartPos;
            while(markPos < maxSize)
            {
                painter->drawLine(0, static_cast<int>(markPos), width(), static_cast<int>(markPos));
                double time = (round(((markPos - m_yOffset)*inverseSpacing) * unitDivisor) * zoomMultiple);

                painter->drawText(2, static_cast<int>(markPos)+ 1, QString::number(time * -1));
                //painter.drawText(static_cast<int>(markPos)+ 1,10, QString::number((((markPos - m_origin)*inverseSpacing) * unitDivisor) * zoomMultiple));
                markPos += markSpacing;
            }
        }

    }


    m_pathsDirty = false;

    if(m_effectBands.isEmpty() && m_channelBand.isEmpty())
        return;

    painter->resetTransform();

    // Draw each cached min/max envelope as a stack of 1px-wide vertical bars, the
    // way the waveform widget does. No path, no per-paint transform copy, no
    // antialiasing — smooth regions collapse to a thin band that reads like a
    // line, dense oscillation fills as a band. Cost is bounded by column count.
    auto drawBand = [painter](const CurveBand &band, const QColor &color, float minThickness){
        for(int c = 0; c < band.topY.size(); ++c)
        {
            float top = band.topY[c];
            float bottom = band.bottomY[c];
            if(top > bottom)
                std::swap(top, bottom);
            float h = bottom - top;
            if(h < minThickness)
            {
                const float mid = (top + bottom) * 0.5f;
                top = mid - minThickness * 0.5f;
                h = minThickness;
            }
            painter->fillRect(QRectF(band.startX + c, top, 1.0, h), color);
        }
    };

    // Composed context, dimmed: the final channel value and the chain up to here.
    drawBand(m_channelBand, QColor(255,255,255,90), 2.0f);
    for(const CurveBand &band : m_effectBands)
        drawBand(band, QColor(140,140,140), 2.0f);

    // The selected effect's own contribution, in the gizmo's colour and on top, so
    // the gizmo visibly rides the shape it controls.
    for(const CurveBand &band : m_selectedBands)
        drawBand(band, QColor(0,220,220,220), 2.0f);
}

void EffectEditorViewer::drawForeground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawForeground(painter, rect);

}


void EffectEditorViewer::resizeEvent(QResizeEvent *t_event)
{
    QWidget::resizeEvent(t_event);

    if(m_yScale < 0)
        fitY();
}

void EffectEditorViewer::remakeTransform()
{
    m_transform = QTransform();
    m_transform.translate(-m_xOffset,m_yOffset);
    m_transform.scale(m_xScale,-m_yScale);

    m_effect->setViewState(ChannelEffectViewState{m_yScale, m_yOffset});
    m_sceneBounds = m_transform.inverted().mapRect(rect().toRectF());

    m_pathsDirty = true;
    m_colorsDirty = true;
    scene()->invalidate();
    emit relayout();
}

QTransform EffectEditorViewer::transform() const
{
    return m_transform;
}

QPointF EffectEditorViewer::viewScale() const
{
    return QPointF(m_xScale, m_yScale);
}

QPointF EffectEditorViewer::viewOffset() const
{
    return QPointF(m_xOffset, m_yOffset);
}

QRectF EffectEditorViewer::sceneBounds() const
{
    return m_sceneBounds;
}

void EffectEditorViewer::fitY()
{
    double initialValue = m_effect->channel()->info().defaultValue.toDouble();

    double startTime = m_effect->channel()->startTime();
    double endTime = m_effect->channel()->endTime();




    double interval = (endTime - startTime)/100.0;

    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::lowest();

    //qDebug() << left << right << (right - left) / interval;

    auto subChannelCount = std::max(m_effect->channel()->subChannelCount(),1);
    float *values = new float[subChannelCount];

    double d = startTime;
    while( d < endTime )
    {
        d += interval;
        values = m_effect->process(values, subChannelCount, d - startTime);

        for(int i = 0; i < subChannelCount; ++i)
        {
            float val = values[i];
            if(val < minValue)
                minValue = val;
            if(val > maxValue)
                maxValue = val;
        }



    }

    double totalHeight = maxValue - minValue;

    if(totalHeight < .0001)
    {
        totalHeight = 2.0;
        maxValue += 1.0;
    }

    double yScale = (height() - 20.0)/totalHeight;
    double yOff = maxValue * yScale;

    qDebug() << totalHeight << yScale << yOff;

    setScale(QPointF{m_xScale, yScale});
    m_yOffset = yOff + 10.0;
    remakeTransform();
}

void EffectEditorViewer::setScale(QPointF t_value)
{
    if(qFuzzyCompare(t_value.x(), m_xScale) && qFuzzyCompare(t_value.y(), m_yScale))
        return;

    if(t_value.x() < 2.0)
        t_value.setX(2.0);
    /*
    if(t_value.y() < .001)
        t_value.setY(.001);
*/

    m_xScale = t_value.x();
    m_yScale = t_value.y();
    remakeTransform();
    emit scaleChanged(t_value);
}

void EffectEditorViewer::setOffset(double t_value)
{
    if(m_xOffset == t_value)
        return;
    m_xOffset = t_value;
    remakeTransform();
    emit offsetChanged(m_xOffset);
}

void EffectEditorViewer::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    m_startPt = event->pos();
    m_lastPt = m_startPt;
    m_startXPos = (m_startPt.x() + m_xOffset) / m_xScale;
    m_startYPos = (m_yOffset - m_startPt.y()) / m_yScale;

    qDebug() << m_yOffset << m_startPt.y() << m_startYPos;
}

void EffectEditorViewer::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    QPoint deltaPt = event->pos() - m_lastPt;

    if(event->buttons() & Qt::MiddleButton)
    {
        m_yOffset += deltaPt.y();
        setOffset(m_xOffset - deltaPt.x());
    }
    if(event->buttons() & Qt::LeftButton && event->modifiers() & Qt::ControlModifier)
    {
        double newScaleY = m_yScale;
        double newScaleX = m_xScale;

        if(deltaPt.y() > 0)
            newScaleY *= 1.04;
        else if(deltaPt.y() < 0)
            newScaleY /= 1.04;
        if(newScaleY < .001)
            newScaleY = .001;

        if(deltaPt.x() > 0)
            newScaleX *= 1.1;
        else if(deltaPt.x() < 0)
            newScaleX /= 1.1;

        if(newScaleX < .001)
            newScaleX = .001;


        setScale(QPointF(newScaleX, newScaleY));

        double newXPos = (m_startPt.x() + m_xOffset) / newScaleX;
        double newYPos = (m_yOffset - m_startPt.y()) / newScaleY;

        //qDebug() << deltaPt.y() << m_startPt.y() << newScaleY << m_yOffset << ((m_startYPos - newYPos) * -newScaleY);


        m_yOffset = ((m_startYPos - newYPos) * newScaleY) + m_yOffset;
        setOffset(m_xOffset - ((newXPos - m_startXPos) * newScaleX) );
        m_startXPos = (m_startPt.x() + m_xOffset) / newScaleX;

        m_startYPos = (m_yOffset - m_startPt.y()) / newScaleY;

        remakeTransform();

    }

    m_lastPt = event->pos();

}

void EffectEditorViewer::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event);

}

void EffectEditorViewer::wheelEvent(QWheelEvent *event)
{

    m_startPt = event->position().toPoint();
    m_lastPt = m_startPt;
    m_startXPos = (m_startPt.x() + m_xOffset) / m_xScale;
    m_startYPos = (m_yOffset - m_startPt.y()) / m_yScale;

    // Some platforms report the wheel on the x axis when a modifier is held (Alt
    // is the horizontal-scroll modifier), so take whichever axis is non-zero.
    const int wheelDelta = event->angleDelta().y() != 0 ? event->angleDelta().y()
                                                        : event->angleDelta().x();

    // Ctrl zooms TIME (x), matching the other views. Alt zooms the VALUE range (y),
    // which is unique to this editor.
    if(event->modifiers() & Qt::ControlModifier)
    {

        double newScaleY = m_yScale;
        double newScaleX = m_xScale;

        if(wheelDelta > 0)
            newScaleX *= 1.24;
        else
            newScaleX /= 1.24;

        setScale(QPointF(newScaleX, newScaleY));

        double newXPos = (m_startPt.x() + m_xOffset) / newScaleX;
        double newYPos = (m_yOffset - m_startPt.y()) / newScaleY;

        m_yOffset = ((m_startYPos - newYPos) * newScaleY) + m_yOffset;
        setOffset(m_xOffset - ((newXPos - m_startXPos) * newScaleX) );
        m_startXPos = (m_startPt.x() + m_xOffset) / newScaleX;
        m_startYPos = (m_yOffset - m_startPt.y()) / newScaleY;

        remakeTransform();
    }

    if(event->modifiers() & Qt::AltModifier)
    {

        double newScaleY = m_yScale;
        double newScaleX = m_xScale;

        if(wheelDelta > 0)
            newScaleY *= 1.24;
        else
            newScaleY /= 1.24;

        setScale(QPointF(newScaleX, newScaleY));

        double newXPos = (m_startPt.x() + m_xOffset) / newScaleX;
        double newYPos = (m_yOffset - m_startPt.y()) / newScaleY;

        m_yOffset = ((m_startYPos - newYPos) * newScaleY) + m_yOffset;
        setOffset(m_xOffset - ((newXPos - m_startXPos) * newScaleX) );
        m_startXPos = (m_startPt.x() + m_xOffset) / newScaleX;
        m_startYPos = (m_yOffset - m_startPt.y()) / newScaleY;

        remakeTransform();
    }

    if(!(event->modifiers() & (Qt::ControlModifier | Qt::AltModifier)))
    {
        // Plain wheel: horizontal pan through setOffset, matching the layer and
        // waveform views and keeping all three in sync.
        const QPoint pixels = event->pixelDelta();
        double delta;
        if(!pixels.isNull())
            delta = pixels.y() != 0 ? pixels.y() : pixels.x();
        else
            delta = (wheelDelta / 120.0) * 40.0;   // ~40px per wheel notch

        setOffset(m_xOffset - delta);
    }
}

void EffectEditorViewer::rebuildColors()
{
    m_colors.clear();

    if(m_effect)
    {
        QColor initialValue = m_effect->channel()->info().defaultValue.value<QColor>();

        double startTime = m_effect->channel()->startTime();
        double endTime = m_effect->channel()->endTime();

        if(startTime > m_sceneBounds.right() || endTime < m_sceneBounds.left() + (1 / m_xScale))
            return;

        double left = std::max(m_sceneBounds.left(), startTime);
        double right = std::min(m_sceneBounds.right(), m_effect->channel()->endTime());

        double interval = (right - left)/width();

        float *values = new float[4];

        ColorChannelParameter::colorToChannels(initialValue, values);

        values = m_effect->process(values, 4, left - startTime);

        m_colors << ColorChannelParameter::channelsToColor(values).rgba();

        //qDebug() << left << right << (right - left) / interval;

        double d = left;
        while( d < right )
        {

            ColorChannelParameter::colorToChannels(initialValue, values);
            d += interval;

            values = m_effect->process(values, 4, d - startTime);
            m_colors << ColorChannelParameter::channelsToColor(values).rgba();
        }


        delete[] values;

    }
}

void EffectEditorViewer::rebuildPaths()
{
    m_effectBands.clear();
    m_selectedBands.clear();
    m_channelBand = CurveBand{};

    if(!m_effect)
        return;

    Channel *channel = m_effect->channel();
    const double initialValue = channel->info().defaultValue.toDouble();
    const double startTime = channel->startTime();
    const double endTime = channel->endTime();

    if(startTime > m_sceneBounds.right() || endTime < m_sceneBounds.left() + (1 / m_xScale))
        return;

    const double left = std::max(m_sceneBounds.left(), startTime);
    const double right = std::min(m_sceneBounds.right(), endTime);
    if(right <= left)
        return;

    // On-screen pixel span of the curve. We build a min/max envelope one entry
    // per pixel column, exactly like the waveform renderer, so the amount of work
    // (and drawn primitives) is bounded by screen resolution regardless of how
    // fast the effect oscillates.
    int startX = static_cast<int>(std::floor(m_transform.map(QPointF(left, 0.0)).x()));
    int endX   = static_cast<int>(std::ceil(m_transform.map(QPointF(right, 0.0)).x()));
    startX = std::clamp(startX, 0, width());
    endX   = std::clamp(endX, 0, width());
    const int columns = endX - startX;
    if(columns <= 0)
        return;

    const QTransform inv = m_transform.inverted();

    // Value -> screen-y. The transform has no shear, so y depends only on value.
    auto valueToY = [this](double v) -> float {
        return static_cast<float>(m_transform.map(QPointF(0.0, v)).y());
    };

    // Sub-sample within each column so oscillation faster than one pixel still
    // registers as a min/max band. Total evals stay within a fixed budget.
    const int kBudget = 4000;
    const int subSamples = std::clamp(kBudget / columns, 2, 16);
    const double subDiv = subSamples - 1;

    // Two bands per subchannel:
    //  - m_effectBands   : the fully composed chain up to and including this effect
    //                      (what actually reaches the output), drawn as context.
    //  - m_selectedBands : just THIS effect's own contribution, i.e. the chain with
    //                      and without it, differenced and re-centred on zero. That
    //                      is the curve the gizmo describes (e.g. a sine of the set
    //                      amplitude around 0), so drawing it lets the gizmo ride on
    //                      the shape it controls regardless of what upstream does.
    // Isolating this way (rather than detaching previousEffect) keeps us off the
    // shared effect state the eval thread may be using.
    const int subChannelCount = std::max(channel->subChannelCount(), 1);
    const bool wantIsolated = m_effect->providesIsolatedContribution();
    ChannelEffect *previous = m_effect->previousEffect();

    m_effectBands.resize(subChannelCount);
    for(CurveBand &band : m_effectBands)
    {
        band.startX = startX;
        band.topY.resize(columns);
        band.bottomY.resize(columns);
    }
    if(wantIsolated)
    {
        m_selectedBands.resize(subChannelCount);
        for(CurveBand &band : m_selectedBands)
        {
            band.startX = startX;
            band.topY.resize(columns);
            band.bottomY.resize(columns);
        }
    }

    float *fullValues = new float[subChannelCount];
    float *upstreamValues = new float[subChannelCount];
    std::vector<double> colMin(subChannelCount);
    std::vector<double> colMax(subChannelCount);
    std::vector<double> selMin(subChannelCount);
    std::vector<double> selMax(subChannelCount);

    for(int c = 0; c < columns; ++c)
    {
        const int x = startX + c;
        const double tL = inv.map(QPointF(x, 0.0)).x();
        const double tR = inv.map(QPointF(x + 1, 0.0)).x();

        for(int k = 0; k < subChannelCount; ++k)
        {
            colMin[k] = std::numeric_limits<double>::max();
            colMax[k] = std::numeric_limits<double>::lowest();
            selMin[k] = std::numeric_limits<double>::max();
            selMax[k] = std::numeric_limits<double>::lowest();
        }

        for(int s = 0; s < subSamples; ++s)
        {
            const double t = tL + (tR - tL) * (s / subDiv);
            const double localTime = t - startTime;

            for(int k = 0; k < subChannelCount; ++k)
                fullValues[k] = initialValue;
            fullValues = m_effect->process(fullValues, subChannelCount, localTime);

            if(wantIsolated && previous)
            {
                for(int k = 0; k < subChannelCount; ++k)
                    upstreamValues[k] = initialValue;
                upstreamValues = previous->process(upstreamValues, subChannelCount, localTime);
            }

            for(int k = 0; k < subChannelCount; ++k)
            {
                const double full = fullValues[k];
                if(std::isfinite(full))
                {
                    colMin[k] = std::min(colMin[k], full);
                    colMax[k] = std::max(colMax[k], full);
                }

                if(wantIsolated)
                {
                    // This effect's own contribution, centred on zero.
                    const double upstream = previous ? upstreamValues[k] : initialValue;
                    const double isolated = full - upstream;
                    if(std::isfinite(isolated))
                    {
                        selMin[k] = std::min(selMin[k], isolated);
                        selMax[k] = std::max(selMax[k], isolated);
                    }
                }
            }
        }

        for(int k = 0; k < subChannelCount; ++k)
        {
            if(colMax[k] < colMin[k]) { colMin[k] = 0; colMax[k] = 0; }
            m_effectBands[k].topY[c]    = valueToY(colMax[k]);
            m_effectBands[k].bottomY[c] = valueToY(colMin[k]);

            if(wantIsolated)
            {
                if(selMax[k] < selMin[k]) { selMin[k] = 0; selMax[k] = 0; }
                m_selectedBands[k].topY[c]    = valueToY(selMax[k]);
                m_selectedBands[k].bottomY[c] = valueToY(selMin[k]);
            }
        }
    }
    delete[] fullValues;
    delete[] upstreamValues;

    // ---- channel value band ----
    const auto channelType = channel->info().type;
    if(channelType == ChannelInfo::ChannelTypeBool ||
       channelType == ChannelInfo::ChannelTypeInteger ||
       channelType == ChannelInfo::ChannelTypeIntegerStep ||
       channelType == ChannelInfo::ChannelTypeNumber)
    {
        m_channelBand.startX = startX;
        m_channelBand.topY.resize(columns);
        m_channelBand.bottomY.resize(columns);

        for(int c = 0; c < columns; ++c)
        {
            const int x = startX + c;
            const double tL = inv.map(QPointF(x, 0.0)).x();
            const double tR = inv.map(QPointF(x + 1, 0.0)).x();

            double mn = std::numeric_limits<double>::max();
            double mx = std::numeric_limits<double>::lowest();
            for(int s = 0; s < subSamples; ++s)
            {
                const double t = tL + (tR - tL) * (s / subDiv);
                const double v = channel->processValue(t - startTime).toDouble();
                if(!std::isfinite(v))
                    continue;
                mn = std::min(mn, v);
                mx = std::max(mx, v);
            }
            if(mx < mn) { mn = 0; mx = 0; }
            m_channelBand.topY[c]    = valueToY(mx);
            m_channelBand.bottomY[c] = valueToY(mn);
        }
    }
}


ChannelEffectEditor::ChannelEffectEditor(ChannelEffect *t_effect, QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    m_impl->effect = t_effect;
    m_impl->toolbar = new QToolBar;
    m_impl->viewer = new EffectEditorViewer(t_effect);
    m_impl->scene = new EffectEditorScene;
    //m_impl->scene->setSceneRect(0,-100,300,200);
    m_impl->viewer->setScene(m_impl->scene);


    m_impl->toolbar->addAction("Y Fit",[this](){fitY();});

    setMinimumHeight(100);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->setSpacing(0);
    vLayout->addWidget(m_impl->viewer);
    vLayout->addWidget(m_impl->toolbar);
    setLayout(vLayout);


    connect(t_effect->channel(), &Channel::channelUpdated, this, &ChannelEffectEditor::channelUpdated);
    connect(t_effect->channel(), &Channel::effectModified, this, &ChannelEffectEditor::effectUpdated);
    connect(m_impl->viewer, &EffectEditorViewer::relayout, this, &ChannelEffectEditor::relayoutSlot);
    connect(m_impl->viewer, &EffectEditorViewer::scaleChanged, this, &ChannelEffectEditor::scaleChanged);
    connect(m_impl->viewer, &EffectEditorViewer::offsetChanged, this, &ChannelEffectEditor::offsetChanged);
    //setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
}

void ChannelEffectEditor::resizeEvent(QResizeEvent *t_event)
{
    QWidget::resizeEvent(t_event);

    m_impl->scene->setSceneRect(QRect(QPoint(0,0),t_event->size()));
    m_impl->viewer->remakeTransform();

    QPoint delta{t_event->size().width() - t_event->oldSize().width(), 0};

    for(auto container : m_impl->containers)
    {
        if(!container->isCustomLayout())
        {
            auto r = container->rect();
            container->setPos(QPointF(t_event->size().width() - (r.width() + 10), 10));
        }
        else
            container->setPos(container->pos() + delta);
    }
}

void ChannelEffectEditor::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void ChannelEffectEditor::fitY()
{
    m_impl->viewer->fitY();
}

ChannelEffectEditor::~ChannelEffectEditor()
{
    // If the Properties panel is still showing our widgets (wired to this editor),
    // clear it so the Project deletes our container before we're gone.
    if(m_impl->propertiesContainer &&
       photonApp->project()->propertiesWidget() == m_impl->propertiesContainer)
        photonApp->project()->setPropertiesWidget(nullptr);

    delete m_impl;
}

void ChannelEffectEditor::addItem(QGraphicsItem *t_item)
{
    m_impl->scene->addItem(t_item);
}

void ChannelEffectEditor::addWidget(QWidget *t_widget, const QString &t_name)
{
    // Property widgets go to the shared Properties panel rather than a floating box
    // on the curve, keeping the canvas to just the waveform + gizmos.
    if(!m_impl->propertiesContainer)
    {
        m_impl->propertiesContainer = new QWidget;
        auto *containerLayout = new QVBoxLayout(m_impl->propertiesContainer);
        containerLayout->setContentsMargins(4,4,4,4);
        containerLayout->addStretch(1);
    }

    auto *group = new QGroupBox(t_name);
    auto *groupLayout = new QVBoxLayout(group);
    groupLayout->setContentsMargins(6,6,6,6);
    groupLayout->addWidget(t_widget);

    auto *containerLayout = static_cast<QVBoxLayout*>(m_impl->propertiesContainer->layout());
    containerLayout->insertWidget(containerLayout->count() - 1, group);   // before the trailing stretch

    photonApp->project()->setPropertiesWidget(m_impl->propertiesContainer);
}

void ChannelEffectEditor::removeWidget(QWidget *t_widget)
{
    for(auto container : m_impl->containers)
    {
        if(container->widget() == t_widget)
        {
            m_impl->containers.removeOne(container);
            delete container;
            return;
        }
    }
}

QTransform ChannelEffectEditor::transform() const
{
    return m_impl->viewer->transform();
}

QPointF ChannelEffectEditor::scale() const
{
    return m_impl->viewer->viewScale();
}

QPointF ChannelEffectEditor::offset() const
{
    return m_impl->viewer->viewOffset();
}

ChannelEffect *ChannelEffectEditor::effect() const
{
    return m_impl->effect;
}

QToolBar *ChannelEffectEditor::toolbar() const
{
    return m_impl->toolbar;
}

QGraphicsScene *ChannelEffectEditor::scene() const
{
    return m_impl->scene;
}

QGraphicsView *ChannelEffectEditor::view() const
{
    return m_impl->viewer;
}

void ChannelEffectEditor::effectUpdated(photon::ChannelEffect *t_effect)
{
    if(t_effect == m_impl->effect)
    {
        m_impl->viewer->remakeTransform();
    }
    else
    {
        m_impl->scene->invalidate();
    }
}

void ChannelEffectEditor::setOffset(double t_value)
{
    m_impl->offset = t_value;
    m_impl->viewer->setOffset(t_value);
}

void ChannelEffectEditor::setXScale(double t_value)
{
    m_impl->scale.setX(t_value);
    m_impl->viewer->setScale(QPointF(t_value, m_impl->viewer->scale().y()));
}

void ChannelEffectEditor::setScale(QPointF t_value)
{
    m_impl->scale = t_value;
    m_impl->viewer->setScale(t_value);
}

void ChannelEffectEditor::channelUpdated(Channel *)
{
     m_impl->viewer->remakeTransform();
}

void ChannelEffectEditor::relayoutSlot()
{
    relayout(m_impl->viewer->sceneBounds());
}

void ChannelEffectEditor::relayout(const QRectF &t_rect)
{

}


} // namespace photon
