#include <QVBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include "surfacewidget.h"
#include "surface/surface.h"
#include "surface/surfacegizmocontainer.h"
#include "surfacegraphwidget.h"
#include "surfacegizmocontainerwidget.h"
#include "util/utils.h"

namespace photon {

class SurfaceWidget::Impl
{
public:
    Impl(SurfaceWidget *);
    void buildLayout();
    SurfaceWidget *facade;
    QWidget *surfaceContainer;
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
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setContentsMargins(0,0,0,0);
    hLayout->addWidget(m_impl->surfaceContainer);

    setLayout(hLayout);


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
