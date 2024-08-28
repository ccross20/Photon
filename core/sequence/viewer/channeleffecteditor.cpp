#include <QPainter>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QGraphicsLinearLayout>
#include "channeleffecteditor_p.h"
#include "sequence/channeleffect.h"
#include "sequence/channel.h"
#include "sequence/clip.h"
#include "sequence/viewer/stackedparameterwidget.h"
#include "channel/parameter/colorchannelparameter.h"

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

    if(m_path.isEmpty())
        return;


    painter->resetTransform();

    painter->setRenderHint(QPainter::Antialiasing);

    painter->setPen(QPen(QColor(255,255,255,127), 2));
    painter->drawPath(m_transform.map(m_channelPath));


    painter->setPen(QPen(QColor(200,200,200), 2));
    painter->drawPath(m_transform.map(m_path));
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

    if(event->modifiers() & Qt::ControlModifier)
    {

        double newScaleY = m_yScale;
        double newScaleX = m_xScale;

        if(event->angleDelta().y() > 0)
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

    if(event->modifiers() & Qt::AltModifier)
    {

        double newScaleY = m_yScale;
        double newScaleX = m_xScale;

        if(event->angleDelta().x() > 0)
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
    m_path.clear();
    m_channelPath.clear();

    if(m_effect)
    {
        double initialValue = m_effect->channel()->info().defaultValue.toDouble();

        double startTime = m_effect->channel()->startTime();
        double endTime = m_effect->channel()->endTime();

        if(startTime > m_sceneBounds.right() || endTime < m_sceneBounds.left() + (1 / m_xScale))
            return;

        double left = std::max(m_sceneBounds.left(), startTime);
        double right = std::min(m_sceneBounds.right(), m_effect->channel()->endTime());

        double interval = (right - left)/width();

        auto subChannelCount = std::max(m_effect->channel()->subChannelCount(),1);

        if(subChannelCount > 0)
        {
            float *tempValues = new float[subChannelCount];
            std::vector<std::vector<float>> values(width()+5,std::vector<float>(subChannelCount));

            for(int v = 0; v < subChannelCount; ++v)
            {
                tempValues[v] = initialValue;
            }

            tempValues = m_effect->process(tempValues,subChannelCount,left-startTime);

            for(int v = 0; v < subChannelCount; ++v)
            {
                values[0][v] = tempValues[v];
            }


            //qDebug() << left << right << (right - left) / interval;
            int counter = 1;
            double d = left;
            while( d < right )
            {
                for(int v = 0; v < subChannelCount; ++v)
                {
                    tempValues[v] = initialValue;
                }

                d += interval;
                tempValues = m_effect->process(tempValues,subChannelCount, d - startTime);

                for(int v = 0; v < subChannelCount; ++v)
                {
                    values[counter][v] = tempValues[v];
                }
                counter++;
            }

            delete[] tempValues;


            for(int k = 0; k < subChannelCount; ++k)
            {
                m_path.moveTo(left,values[0][k]);
                for(int i = 1; i < width(); ++i)
                {

                    m_path.lineTo(left,values[i][k]);


                }
            }
        }




        auto channel = m_effect->channel();
        auto channelType = channel->info().type;

        if(channelType == ChannelInfo::ChannelTypeBool ||
            channelType == ChannelInfo::ChannelTypeInteger ||
            channelType == ChannelInfo::ChannelTypeIntegerStep ||
            channelType == ChannelInfo::ChannelTypeNumber)
        {
            interval = (right - left)/width();
            m_channelPath.moveTo(left,channel->processValue(left - startTime).toDouble());

            double d = left;
            while( d < right )
            {
                d += interval;
                m_channelPath.lineTo(d, channel->processValue(d - startTime).toDouble());
            }
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
    delete m_impl;
}

void ChannelEffectEditor::addItem(QGraphicsItem *t_item)
{
    m_impl->scene->addItem(t_item);
}

void ChannelEffectEditor::addWidget(QWidget *t_widget, const QString &t_name)
{
    WidgetContainer *container = new WidgetContainer(t_widget, t_name);
    m_impl->scene->addItem(container);
    container->addedToScene(m_impl->scene);

    m_impl->containers.append(container);

    auto r = container->rect();

    container->setPos(QPointF(width() -(r.width() + 10), 10));
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
