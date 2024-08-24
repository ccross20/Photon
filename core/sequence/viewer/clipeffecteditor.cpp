#include <QPainter>
#include <QResizeEvent>
#include "clipeffecteditor.h"
#include "sequence/channeleffect.h"
#include "sequence/channel.h"
#include "sequence/clip.h"

namespace photon {



CurveViewport::CurveViewport()
{
    m_viewBounds = QRectF(0,1.0,300,2.0);
}

void CurveViewport::setViewBounds(const QRectF &rect)
{
    m_viewBounds = rect;
    m_pathsDirty = true;
    update();
}

void CurveViewport::clear()
{
    m_effect = nullptr;
    m_path.clear();
    m_channelPath.clear();
    update();
}

void CurveViewport::clipChanged(Clip *)
{
    m_pathsDirty = true;
    update();
}

void CurveViewport::channelUpdated(photon::Channel* t_channel)
{
    if(t_channel == m_effect->channel())
    {
        m_pathsDirty = true;
        update();
    }

}

void CurveViewport::remakeSceneBounds()
{
    m_sceneBounds = QRectF(m_offset/m_scale,0,m_viewBounds.width()/m_scale, 2);
    m_pathsDirty = true;
    update();
}

void CurveViewport::resizeEvent(QResizeEvent *t_event)
{
    m_viewBounds = QRectF(0,0,t_event->size().width(), t_event->size().height());
    remakeSceneBounds();
}

void CurveViewport::setXOffset(int value)
{
    m_offset = value;
    remakeSceneBounds();
}

void CurveViewport::rebuildPaths()
{
    m_path.clear();
    m_channelPath.clear();

    if(m_effect)
    {
        double initialValue = m_effect->channel()->info().defaultValue.toDouble();

        double startTime = m_effect->channel()->startTime();

        double left = std::max(m_sceneBounds.left(), startTime);
        double right = std::min(m_sceneBounds.right(), m_effect->channel()->endTime());
/*

        double interval = 1.0/m_scale;
        m_path.moveTo(left,*m_effect->process(initialValue, left - startTime));

        double d = left;
        while( d < right )
        {
            d += interval;
            m_path.lineTo(d, m_effect->process(initialValue, d - startTime));
        }

        auto channel = m_effect->channel();

        interval = 1.0/m_scale;
        m_channelPath.moveTo(left,channel->processDouble(left - startTime));

        d = left;
        while( d < right )
        {
            d += interval;
            m_channelPath.lineTo(d, channel->processDouble(d - startTime));
        }
*/
    }
}

void CurveViewport::addEffect(photon::ChannelEffect *t_effect)
{
    if(m_channel)
    {
        disconnect(m_channel, &Channel::channelUpdated, this, &CurveViewport::channelUpdated);
    }
    m_effect = t_effect;
    if(m_effect)
    {
        m_channel = m_effect->channel();
        connect(m_channel, &Channel::channelUpdated, this, &CurveViewport::channelUpdated);
        m_pathsDirty = true;
    }
    else
        m_channel = nullptr;

    update();
}

void CurveViewport::setScale(double t_scale)
{
    m_scale = t_scale;
    remakeSceneBounds();
}

void CurveViewport::paintEvent(QPaintEvent *t_event)
{
    QWidget::paintEvent(t_event);


    if(m_pathsDirty)
        rebuildPaths();

    m_pathsDirty = false;

    if(m_path.isEmpty())
        return;

    QTransform t;
    t.translate(-m_offset, -m_sceneBounds.y());
    t.scale(m_scale,height()/m_sceneBounds.height());

    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(QColor(255,0,0,127), 2));
    painter.drawPath(t.map(m_channelPath));


    painter.setPen(QPen(Qt::red, 2));
    painter.drawPath(t.map(m_path));
}


ClipEffectEditor::ClipEffectEditor(QWidget *parent)
    : QWidget{parent}
{
    m_viewport = new CurveViewport;

    m_layout = new QVBoxLayout;
    m_layout->addWidget(m_viewport);

    setLayout(m_layout);
}

void ClipEffectEditor::setScale(double t_scale)
{
    m_viewport->setScale(t_scale);
}

void ClipEffectEditor::selectEffect(photon::ChannelEffect *t_effect)
{
    if(m_effectEditor)
        delete m_effectEditor;
    m_viewport->addEffect(t_effect);
    m_effectEditor = t_effect->createEditor();
    m_layout->addWidget(m_effectEditor);
}

void ClipEffectEditor::clearSelection()
{
    m_viewport->clear();
    delete m_effectEditor;
    m_effectEditor = nullptr;
}

void ClipEffectEditor::setXOffset(int value)
{
    m_viewport->setXOffset(value);
}

} // namespace photon
