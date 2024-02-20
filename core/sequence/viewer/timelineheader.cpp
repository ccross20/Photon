#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QPushButton>
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
    QPushButton *editButton;
    QVBoxLayout *subLayerLayout;
};



LayerGroupHeader::LayerGroupHeader(LayerGroup *t_group) : LayerHeader(t_group),m_impl(new Impl)
{
    m_impl->subLayerLayout = new QVBoxLayout;
    m_impl->subLayerLayout->setContentsMargins(20,0,0,0);

    connect(t_group, &LayerGroup::layerAdded,this,&LayerGroupHeader::layerAdded);
    connect(t_group, &LayerGroup::layerRemoved,this,&LayerGroupHeader::layerRemoved);
    connect(t_group, &LayerGroup::layerUpdated,this,&LayerGroupHeader::layerUpdated);
}

LayerGroupHeader::~LayerGroupHeader()
{
    delete m_impl;
}

void LayerGroupHeader::layerUpdated(photon::Layer *)
{

}

void LayerGroupHeader::layerAdded(photon::Layer *t_layer)
{
    auto header = new LayerHeader(t_layer);
    header->buildLayout();
    m_impl->subLayerLayout->addWidget(header);

    setMaximumHeight(layer()->height());
}

void LayerGroupHeader::layerRemoved(photon::Layer *)
{

}

LayerGroup *LayerGroupHeader::group() const
{
    return static_cast<LayerGroup*>(layer());
}

void LayerGroupHeader::editLayerSlot()
{
    emit editLayer(layer());
}

void LayerGroupHeader::buildLayout()
{

    QVBoxLayout *masterLayout = new QVBoxLayout;
    masterLayout->setContentsMargins(0,0,0,0);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->addWidget(label());

    m_impl->editButton = new QPushButton("Edit");

    connect(m_impl->editButton, &QPushButton::clicked, this, &LayerGroupHeader::editLayerSlot);

    hLayout->addWidget(m_impl->editButton);

    masterLayout->addLayout(hLayout);


    for(auto layer : group()->layers())
    {
        auto header = new LayerHeader(layer);
        header->buildLayout();
        m_impl->subLayerLayout->addWidget(header);
    }

    masterLayout->addLayout(m_impl->subLayerLayout);


    //m_impl->subLayerLayout->addStretch();
    setLayout(masterLayout);
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

        connect(header, SIGNAL(editLayer(photon::Layer *)), this, SIGNAL(editLayer(photon::Layer *)));
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
        disconnect(header, SIGNAL(editLayer()), this, SIGNAL(editLayer()));
        m_impl->headers.erase(it);
        m_impl->vLayout->removeWidget(header);
    }
}

} // namespace photon
