#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QButtonGroup>
#include "surfacewidget.h"
#include "surface/surface.h"
#include "surface/surfacegizmocontainer.h"
#include "surfacegraphwidget.h"
#include "surfacegizmocontainerwidget.h"
#include "surfacequickview.h"
#include "util/utils.h"

namespace photon {

class SurfaceWidget::Impl
{
public:
    Impl(SurfaceWidget *);
    void buildLayout();
    SurfaceWidget *facade;
    QWidget *surfaceContainer;
    QStackedWidget *stack = nullptr;
    SurfaceQuickView *quickView = nullptr;
    Surface *surface = nullptr;
    SurfaceMode mode = Mode_Edit;
    QVBoxLayout *vLayout = nullptr;
};

void SurfaceWidget::Impl::buildLayout()
{
    if(vLayout)
        clearLayout(surfaceContainer->layout());
    else
    {
        vLayout = new QVBoxLayout;
        vLayout->setContentsMargins(0,0,0,0);
        surfaceContainer->setLayout(vLayout);
    }

    for (auto container : surface->containers()) {
        vLayout->addWidget(container->createWidget(mode));
    }

    QPushButton *emptyButton = new QPushButton("Add Container");
    emptyButton->setMaximumHeight(10000);
    connect(emptyButton, &QPushButton::clicked,surface, [this](){
        surface->addGizmoContainer(new SurfaceGizmoContainer);
    });
    vLayout->addWidget(emptyButton);
}

SurfaceWidget::Impl::Impl(SurfaceWidget *t_facade):facade(t_facade)
{
    surfaceContainer = new QWidget;
}

SurfaceWidget::SurfaceWidget(QWidget *parent)
    : QWidget{parent},m_impl(new Impl(this))
{
    m_impl->quickView = new SurfaceQuickView;

    m_impl->stack = new QStackedWidget;
    m_impl->stack->addWidget(m_impl->surfaceContainer); // index 0: edit
    m_impl->stack->addWidget(m_impl->quickView);        // index 1: perform

    // Mode toolbar
    auto *editButton = new QPushButton("Edit");
    auto *performButton = new QPushButton("Perform");
    editButton->setCheckable(true);
    performButton->setCheckable(true);
    editButton->setChecked(true);

    auto *group = new QButtonGroup(this);
    group->setExclusive(true);
    group->addButton(editButton);
    group->addButton(performButton);

    connect(editButton, &QPushButton::clicked, this, [this](){ setMode(Mode_Edit); });
    connect(performButton, &QPushButton::clicked, this, [this](){ setMode(Mode_Performance); });

    auto *toolbar = new QHBoxLayout;
    toolbar->setContentsMargins(4,4,4,4);
    toolbar->addWidget(editButton);
    toolbar->addWidget(performButton);
    toolbar->addStretch();

    QVBoxLayout *rootLayout = new QVBoxLayout;
    rootLayout->setContentsMargins(0,0,0,0);
    rootLayout->addLayout(toolbar);
    rootLayout->addWidget(m_impl->stack);

    setLayout(rootLayout);
}

SurfaceWidget::~SurfaceWidget()
{
    delete m_impl;
}

void SurfaceWidget::setSurface(Surface *t_surface)
{
    if(m_impl->surface)
    {
        if(m_impl->surface == t_surface)
            return;
        disconnect(m_impl->surface, &Surface::surfaceWasModified, this, &SurfaceWidget::rebuildLayout);
    }
    m_impl->surface = t_surface;
    connect(m_impl->surface, &Surface::surfaceWasModified, this, &SurfaceWidget::rebuildLayout);
    m_impl->quickView->setSurface(t_surface);
    m_impl->buildLayout();
}

Surface *SurfaceWidget::surface() const
{
    return m_impl->surface;
}

SurfaceMode SurfaceWidget::mode() const
{
    return m_impl->mode;
}

void SurfaceWidget::setMode(SurfaceMode t_mode)
{
    m_impl->mode = t_mode;
    m_impl->stack->setCurrentIndex(t_mode == Mode_Performance ? 1 : 0);
    if(t_mode == Mode_Performance)
        m_impl->quickView->setSurface(m_impl->surface);
    emit modeChanged(t_mode);
}

void SurfaceWidget::rebuildLayout()
{
    m_impl->buildLayout();
}

void SurfaceWidget::showEvent(QShowEvent *t_event)
{
    QWidget::showEvent(t_event);
}

void SurfaceWidget::resizeEvent(QResizeEvent *t_event)
{
    QWidget::resizeEvent(t_event);
}


} // namespace photon
