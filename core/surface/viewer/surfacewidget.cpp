#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStackedWidget>
#include <QButtonGroup>
#include "surfacewidget.h"
#include "surface/surface.h"
#include "surfacequickview.h"

namespace photon {

class SurfaceWidget::Impl
{
public:
    Impl(SurfaceWidget *t_facade):facade(t_facade){}
    SurfaceWidget *facade;
    QStackedWidget *stack = nullptr;
    SurfaceQuickView *editView = nullptr;
    SurfaceQuickView *performView = nullptr;
    Surface *surface = nullptr;
    SurfaceMode mode = Mode_Edit;
};

SurfaceWidget::SurfaceWidget(QWidget *parent)
    : QWidget{parent},m_impl(new Impl(this))
{
    m_impl->editView = new SurfaceQuickView(true);
    m_impl->performView = new SurfaceQuickView(false);

    m_impl->stack = new QStackedWidget;
    m_impl->stack->addWidget(m_impl->editView);     // index 0: edit (QML designer)
    m_impl->stack->addWidget(m_impl->performView);  // index 1: perform

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
    m_impl->surface = t_surface;
    m_impl->editView->setSurface(t_surface);
    m_impl->performView->setSurface(t_surface);
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
    emit modeChanged(t_mode);
}

void SurfaceWidget::rebuildLayout()
{
    // Views refresh themselves from the surface model; nothing to rebuild here.
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
