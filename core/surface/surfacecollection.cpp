#include "surfacecollection.h"
#include "surface.h"
#include "gui/panel/surfacepanel.h"
#include "photoncore.h"

namespace photon {

class SurfaceCollection::Impl
{
public:
    Impl(SurfaceCollection *);

    QHash<Surface*,SurfacePanel*> panels;
    QVector<Surface *> surfaces;
    SurfaceCollection *facade;
    Surface *activeSurface = nullptr;
    SurfacePanel *activePanel = nullptr;
    quint32 revision = 0;
};

SurfaceCollection::Impl::Impl(SurfaceCollection *t_facade):facade(t_facade)
{

}

SurfaceCollection::SurfaceCollection(QObject *parent)
    : QObject{parent}, m_impl(new Impl(this))
{

}

SurfaceCollection::~SurfaceCollection()
{
    for(auto surface : m_impl->surfaces)
        delete surface;
    delete m_impl;
}

Surface *SurfaceCollection::activeSurface() const
{
    return m_impl->activeSurface;
}

void SurfaceCollection::setActiveSurfacePanel(SurfacePanel *panel)
{
    m_impl->activePanel = panel;

    if(panel)
        photonApp->gui()->bringPanelToFront(panel);
}

SurfacePanel *SurfaceCollection::activeSurfacePanel() const
{
    return m_impl->activePanel;
}

void SurfaceCollection::editSurface(Surface *t_surface)
{
    m_impl->activeSurface = t_surface;
    if(m_impl->panels.contains(t_surface))
    {
        setActiveSurfacePanel(m_impl->panels.value(t_surface));
    }
    else
    {
        SurfacePanel *surfacePanel = static_cast<SurfacePanel*>(photonApp->gui()->createDockedPanel("photon.surface", GuiManager::CenterDockWidgetArea, true));
        surfacePanel->setSurface(t_surface);
        surfacePanel->setName(t_surface->name());
        m_impl->panels.insert(t_surface, surfacePanel);
        connect(surfacePanel, &QObject::destroyed,this, &SurfaceCollection::panelDestroyed);

        setActiveSurfacePanel(surfacePanel);
    }
}

void SurfaceCollection::panelDestroyed(QObject *t_object)
{

    for(auto it = m_impl->panels.constBegin(); it != m_impl->panels.constEnd(); ++it)
    {
        if(it.value() == static_cast<SurfacePanel*>(t_object))
        {
            //removeSurface(it.key());
            m_impl->panels.remove(it.key());
            break;
        }
    }


    setActiveSurfacePanel(nullptr);

}

void SurfaceCollection::clear()
{
    for(auto surface : m_impl->surfaces)
        delete surface;

    m_impl->surfaces.clear();
    ++m_impl->revision;
}

quint32 SurfaceCollection::revision() const
{
    return m_impl->revision;
}

const QVector<Surface*> &SurfaceCollection::surfaces() const
{
    return m_impl->surfaces;
}

int SurfaceCollection::surfaceCount() const
{
    return m_impl->surfaces.length();
}

Surface *SurfaceCollection::surfaceAtIndex(uint t_index) const
{
    return m_impl->surfaces.at(t_index);
}

Surface *SurfaceCollection::findSurfaceWithId(const QByteArray &t_id) const
{
    for(auto seq : m_impl->surfaces)
    {
        if(seq->uniqueId() == t_id)
            return seq;
    }
    return nullptr;
}

void SurfaceCollection::addSurface(photon::Surface *t_surface)
{
    if(m_impl->surfaces.contains(t_surface))
        return;
    emit surfaceWillBeAdded(t_surface, m_impl->surfaces.length());
    m_impl->surfaces.append(t_surface);
    ++m_impl->revision;
    emit surfaceWasAdded(t_surface, m_impl->surfaces.length()-1);
}

void SurfaceCollection::removeSurface(photon::Surface *t_surface)
{
    if(!m_impl->surfaces.contains(t_surface))
        return;

    int index = m_impl->surfaces.indexOf(t_surface);

    // The collection owns its surfaces, so removal destroys. Close any open
    // editor panel first — it holds a bare pointer to the surface and would
    // dangle otherwise. SurfaceNodes referencing this surface resolve by id,
    // so they simply stop finding it and fall back to passing DMX through.
    if(auto *panel = m_impl->panels.take(t_surface))
        panel->deleteLater();

    if(m_impl->activeSurface == t_surface)
        m_impl->activeSurface = nullptr;

    emit surfaceWillBeRemoved(t_surface, index);
    m_impl->surfaces.removeOne(t_surface);
    ++m_impl->revision;
    emit surfaceWasRemoved(t_surface, index);

    t_surface->deleteLater();
}

} // namespace photon

