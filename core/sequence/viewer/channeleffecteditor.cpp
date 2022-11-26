#include <QPainter>
#include <QResizeEvent>
#include <QVBoxLayout>
#include "channeleffecteditor_p.h"
#include "sequence/channeleffect.h"
#include "sequence/channel.h"
#include "sequence/clip.h"

namespace photon {

EffectEditorViewer::EffectEditorViewer(ChannelEffect *t_effect):QGraphicsView(),m_effect(t_effect)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setInteractive(true);
    //setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

}

void EffectEditorViewer::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);
    painter->fillRect(rect, QColor(50,50,50));
    if(m_pathsDirty)
        rebuildPaths();


    painter->setPen(QPen(Qt::white, 0));




    painter->drawLine(m_transform.map(QPointF(0,0)),m_transform.map(QPointF(width(),0)));



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

void EffectEditorViewer::remakeTransform()
{
    m_transform = QTransform();
    m_transform.translate(-m_xOffset,m_yOffset);
    m_transform.scale(m_xScale,m_yScale);


    m_sceneBounds = m_transform.inverted().mapRect(rect());

    m_pathsDirty = true;
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

void EffectEditorViewer::setScale(double t_value)
{
    m_xScale = t_value;
    remakeTransform();
}

void EffectEditorViewer::setOffset(double t_value)
{
    m_xOffset = t_value;
    remakeTransform();
}

void EffectEditorViewer::mousePressEvent(QMouseEvent *event)
{
    QGraphicsView::mousePressEvent(event);
    m_startPt = event->pos();
    m_lastPt = m_startPt;
}

void EffectEditorViewer::mouseMoveEvent(QMouseEvent *event)
{
    QGraphicsView::mouseMoveEvent(event);
    QPoint deltaPt = event->pos() - m_lastPt;

    if(event->buttons() & Qt::MiddleButton)
    {
        m_yOffset += deltaPt.y();
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
    if(event->modifiers() & Qt::ControlModifier)
    {
        m_yScale += event->angleDelta().y()/5.0;

        m_yScale = std::min(m_yScale, -.1);
        remakeTransform();
    }
}



void EffectEditorViewer::rebuildPaths()
{
    m_path.clear();
    m_channelPath.clear();

    if(m_effect)
    {
        double initialValue = m_effect->channel()->info().defaultValue.toDouble();

        double startTime = m_effect->channel()->clip()->startTime();

        double left = std::max(m_sceneBounds.left(), startTime);
        double right = std::min(m_sceneBounds.right(), m_effect->channel()->clip()->endTime());


        double interval = (right - left)/width();
        m_path.moveTo(left,m_effect->process(initialValue, left - startTime));

        //qDebug() << left << right << (right - left) / interval;

        double d = left;
        while( d < right )
        {
            d += interval;
            m_path.lineTo(d, m_effect->process(initialValue, d - startTime));
        }

        auto channel = m_effect->channel();

        interval = (right - left)/width();
        m_channelPath.moveTo(left,channel->processDouble(left - startTime));

        d = left;
        while( d < right )
        {
            d += interval;
            m_channelPath.lineTo(d, channel->processDouble(d - startTime));
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


    setMinimumHeight(100);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->setSpacing(0);
    vLayout->addWidget(m_impl->viewer);
    //vLayout->addWidget(m_impl->toolbar);
    setLayout(vLayout);


    connect(t_effect->channel()->clip(), &Clip::timeChanged, this, &ChannelEffectEditor::clipStartUpdated);
    connect(t_effect->channel()->clip(), &Clip::durationChanged, this, &ChannelEffectEditor::clipDurationUpdated);
    connect(t_effect->channel(), &Channel::effectModified, this, &ChannelEffectEditor::effectUpdated);
    connect(m_impl->viewer, &EffectEditorViewer::relayout, this, &ChannelEffectEditor::relayoutSlot);
    //setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
}

void ChannelEffectEditor::resizeEvent(QResizeEvent *t_event)
{
    QWidget::resizeEvent(t_event);

    m_impl->scene->setSceneRect(QRect(QPoint(0,0),t_event->size()));
    m_impl->viewer->remakeTransform();
}

void ChannelEffectEditor::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

ChannelEffectEditor::~ChannelEffectEditor()
{
    delete m_impl;
}

void ChannelEffectEditor::addItem(QGraphicsItem *t_item)
{
    m_impl->scene->addItem(t_item);
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

void ChannelEffectEditor::setScale(double t_value)
{
    m_impl->scale = t_value;
    m_impl->viewer->setScale(t_value);
}

void ChannelEffectEditor::clipStartUpdated(double)
{
     m_impl->viewer->remakeTransform();
}

void ChannelEffectEditor::clipDurationUpdated(double)
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
