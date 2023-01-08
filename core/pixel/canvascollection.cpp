#include "canvascollection.h"
#include "canvas.h"

namespace photon {

class CanvasCollection::Impl
{
public:
    Impl(CanvasCollection *);

    QVector<Canvas *> canvases;
    CanvasCollection *facade;
};

CanvasCollection::Impl::Impl(CanvasCollection *t_facade):facade(t_facade)
{

}

CanvasCollection::CanvasCollection(QObject *parent)
    : QObject{parent}, m_impl(new Impl(this))
{

}

CanvasCollection::~CanvasCollection()
{
    for(auto canvas : m_impl->canvases)
        delete canvas;
    delete m_impl;
}

const QVector<Canvas*> &CanvasCollection::canvases() const
{
    return m_impl->canvases;
}

int CanvasCollection::canvasCount() const
{
    return m_impl->canvases.length();
}

Canvas *CanvasCollection::canvasAtIndex(uint t_index) const
{
    return m_impl->canvases.at(t_index);
}

void CanvasCollection::addCanvas(photon::Canvas *t_canvas)
{
    if(m_impl->canvases.contains(t_canvas))
        return;
    emit canvasWillBeAdded(t_canvas, m_impl->canvases.length());
    m_impl->canvases.append(t_canvas);
    emit canvasWasAdded(t_canvas, m_impl->canvases.length()-1);
}

void CanvasCollection::removeCanvas(photon::Canvas *t_canvas)
{
    if(!m_impl->canvases.contains(t_canvas))
        return;

    int index = 0;
    for(auto seq : m_impl->canvases)
    {
        if(seq == t_canvas)
            break;
        ++index;
    }

    emit canvasWillBeRemoved(t_canvas, index);
    m_impl->canvases.removeOne(t_canvas);
    emit canvasWasRemoved(t_canvas, index);
}

} // namespace photon
