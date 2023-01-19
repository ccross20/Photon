#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include "sequence/layer.h"
#include "sequence/layergroup.h"
#include "sequence/sequence.h"
#include "timelineheader.h"
#include "photoncore.h"
#include "project/project.h"
#include "pixel/canvascollection.h"
#include "pixel/canvas.h"

namespace photon {

class LayerHeader::Impl
{
public:
    QLabel *label;
    Layer *layer;
};

LayerHeader::LayerHeader(Layer *t_layer):m_impl(new Impl)
{

    m_impl->layer = t_layer;
    QString layerName = t_layer->name();
    if(layerName.isEmpty())
        layerName = "[Unnamed]";
    m_impl->label = new QLabel(layerName);

    setStyleSheet("background-color:rgb(80,80,80);");
    setMinimumHeight(m_impl->layer->height());
    setMaximumHeight(m_impl->layer->height());
}

LayerHeader::~LayerHeader()
{
    delete m_impl;
}

QLabel *LayerHeader::label() const
{
    return m_impl->label;
}

void LayerHeader::buildLayout()
{

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->addWidget(m_impl->label);
    vLayout->addStretch();
    setLayout(vLayout);

}

void LayerHeader::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(event->rect(), QColor(80,80,80));
}

QSize LayerHeader::sizeHint() const
{
    return QSize{20, m_impl->layer->height()};
}

Layer *LayerHeader::layer() const
{
    return m_impl->layer;
}


class LayerGroupHeader::Impl
{
public:
    QComboBox *canvasCombo;
};



LayerGroupHeader::LayerGroupHeader(LayerGroup *t_group) : LayerHeader(t_group),m_impl(new Impl)
{

}

LayerGroupHeader::~LayerGroupHeader()
{
    delete m_impl;
}

LayerGroup *LayerGroupHeader::group() const
{
    return static_cast<LayerGroup*>(layer());
}

void LayerGroupHeader::buildLayout()
{
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0,0,0,0);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->addWidget(label());

    m_impl->canvasCombo = new QComboBox;

    for(auto canvas : photonApp->project()->canvases()->canvases())
        m_impl->canvasCombo->addItem(canvas->name());

    hLayout->addWidget(m_impl->canvasCombo);

    vLayout->addLayout(hLayout);
    vLayout->addStretch();
    setLayout(vLayout);
}







class TimelineHeader::Impl
{
public:
    LayerHeader *findHeader(Layer *t_clip, QVector<LayerHeader*>::const_iterator* t_it = nullptr);
    QVector<LayerHeader*> headers;
    QScrollArea *scrollArea;
    QVBoxLayout *vLayout;
    Sequence *sequence = nullptr;
    int offset = 0;
};


LayerHeader *TimelineHeader::Impl::findHeader(Layer *t_layer, QVector<LayerHeader*>::const_iterator* t_it)
{
    auto result = std::find_if(headers.cbegin(), headers.cend(),[t_layer](LayerHeader *t_testLayer){
                     return t_testLayer->layer() == t_layer;
                 });
    if(t_it)
        *t_it = result;

    if(result != headers.cend())
        return *result;
    return nullptr;
}

TimelineHeader::TimelineHeader(QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    m_impl->vLayout = new QVBoxLayout;
    m_impl->vLayout->setSpacing(2);
    m_impl->vLayout->setContentsMargins(0,0,0,0);


    QVBoxLayout *containerLayout = new QVBoxLayout;
    containerLayout->setContentsMargins(0,0,0,0);
    containerLayout->addLayout(m_impl->vLayout);
    containerLayout->addStretch();
    containerLayout->setSizeConstraint(QLayout::SetNoConstraint);

    m_impl->vLayout->setSizeConstraint(QLayout::SetNoConstraint);

    m_impl->scrollArea = new QScrollArea;

    QWidget *widget = new QWidget();
    widget->setLayout(containerLayout);
    //widget->setMinimumSize(100,300);
    widget->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));

    m_impl->scrollArea->setWidget(widget);
    m_impl->scrollArea->setWidgetResizable(true);
    m_impl->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_impl->scrollArea->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
    widget->setStyleSheet("background-color:rgb(25,25,25);");


    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->addWidget(m_impl->scrollArea);


    setLayout(vLayout);

    setMinimumWidth(100);

}

TimelineHeader::~TimelineHeader()
{
    delete m_impl;
}

void TimelineHeader::setSequence(Sequence *t_sequence)
{
    m_impl->sequence = t_sequence;

    for(auto layer : m_impl->sequence->layers())
        layerAdded(layer);

    connect(m_impl->sequence, &Sequence::layerAdded, this, &TimelineHeader::layerAdded);
    connect(m_impl->sequence, &Sequence::layerUpdated, this, &TimelineHeader::layerUpdated);
    connect(m_impl->sequence, &Sequence::layerRemoved, this, &TimelineHeader::layerRemoved);
}

Sequence *TimelineHeader::sequence() const
{
    return m_impl->sequence;
}

void TimelineHeader::offsetChanged(int t_offset)
{
    m_impl->offset = t_offset;
    m_impl->scrollArea->verticalScrollBar()->setValue(t_offset);
    update();
}

void TimelineHeader::layerUpdated(photon::Layer *)
{

}

void TimelineHeader::layerAdded(photon::Layer *t_layer)
{
    LayerHeader *header = nullptr;
    if(t_layer->isGroup())
    {
        header = new LayerGroupHeader(static_cast<LayerGroup*>(t_layer));
    }
    else
    {
        header = new LayerHeader(t_layer);
    }

    if(!header)
        return;

    header->buildLayout();
    m_impl->vLayout->addWidget(header);
    m_impl->headers.append(header);
}

void TimelineHeader::layerRemoved(photon::Layer *t_layer)
{
    QVector<LayerHeader*>::const_iterator it;
    LayerHeader *header = m_impl->findHeader(t_layer, &it);

    if(header)
    {
        m_impl->headers.erase(it);
        m_impl->vLayout->removeWidget(header);
    }
}

} // namespace photon
