#include <QPainter>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QGraphicsLinearLayout>
#include "channeleffecteditor_p.h"
#include "sequence/channeleffect.h"
#include "sequence/channel.h"
#include "sequence/clip.h"
#include "sequence/viewer/stackedparameterwidget.h"

namespace photon {



WidgetContainer::WidgetContainer(QWidget *t_widget, const QString &t_title):QGraphicsWidget(),m_widget(t_widget),m_title(t_title)
{

    //setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
    m_widget->setStyleSheet("QLabel{color:white;}");

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

}


void EffectEditorViewer::drawBackground(QPainter *painter, const QRectF &rect)
{
    QGraphicsView::drawBackground(painter, rect);
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

                painter->drawText(2, static_cast<int>(markPos)+ 1, QString::number(time));
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

void EffectEditorViewer::remakeTransform()
{
    m_transform = QTransform();
    m_transform.translate(-m_xOffset,m_yOffset);
    m_transform.scale(m_xScale,-m_yScale);


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
    if(event->buttons() & Qt::LeftButton && event->modifiers() & Qt::ControlModifier)
    {
        double y = sqrt(m_yScale);
        y *= 1.0 + (deltaPt.y()/25.0);
        m_yScale = y * y;

        //m_yScale = std::min(m_yScale, -.1);
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
        double y = sqrt(m_yScale);
        y *= 1.0 + (event->angleDelta().y()/360.0);
        m_yScale = y * y;

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

        double startTime = m_effect->channel()->startTime();
        double endTime = m_effect->channel()->endTime();

        if(startTime > m_sceneBounds.right() || endTime < m_sceneBounds.left() + (1 / m_xScale))
            return;

        double left = std::max(m_sceneBounds.left(), startTime);
        double right = std::min(m_sceneBounds.right(), m_effect->channel()->endTime());


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


    connect(t_effect->channel(), &Channel::channelUpdated, this, &ChannelEffectEditor::channelUpdated);
    connect(t_effect->channel(), &Channel::effectModified, this, &ChannelEffectEditor::effectUpdated);
    connect(m_impl->viewer, &EffectEditorViewer::relayout, this, &ChannelEffectEditor::relayoutSlot);
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
