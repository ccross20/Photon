#include "opengllayer.h"
#include "viewport/layer/viewportlayer.h"

namespace exo {

OpenGLLayer::OpenGLLayer(ViewportLayerPtr layer, OpenGLViewport *viewport):m_layer(layer),m_viewport(viewport)
{

    m_layer->addEventListener(this,"OpenGLLayerPainter");
    m_dirtyBounds = m_layer->bounds();
}

OpenGLLayer::~OpenGLLayer()
{

    if(m_layer)
        m_layer->removeEventListener(this);
}

void OpenGLLayer::markDirty()
{
    //m_dirtyBounds = m_layer->bounds();
    m_isDirty = true;
}

void OpenGLLayer::offset(point_d )
{
    markDirty();
}

void OpenGLLayer::scale(double )
{
    markDirty();
}

void OpenGLLayer::rotate(double )
{
    markDirty();
}

void OpenGLLayer::markClean()
{
    m_dirtyBounds = bounds_i{};
    m_isDirty = false;
}

bounds_i OpenGLLayer::dirtyBounds() const
{
    return m_dirtyBounds;
}

OpenGLViewport *OpenGLLayer::viewport() const
{
    return m_viewport;
}

bool OpenGLLayer::isDirty() const
{
    return m_isDirty;
}

void OpenGLLayer::setDirtyBounds(const bounds_i &bounds)
{
    m_dirtyBounds = bounds;
    m_isDirty = bounds.is_valid();


}

void OpenGLLayer::receiveEvent(int eventType, void *source, void *data1, void *data2)
{
    Q_UNUSED(source)
    Q_UNUSED(data2)

    switch (eventType) {
        case Event::Overlay_MarkedDirty:
            setDirtyBounds(m_dirtyBounds.united(bounds_i{*static_cast<bounds_i*>(data1)}));


            if(m_layer->coordinateSpace() == ViewportLayer::SceneSpace)
                m_viewport->update(m_layer->sceneTransform().mapBounds(m_dirtyBounds));
            else
                m_viewport->update(m_dirtyBounds);
            //qDebug() << m_dirtyBounds << " " << m_dirtyBounds.width() << " " << m_dirtyBounds.height() << " " << m_layer->layerId();

            //qDebug() << m_dirtyBounds;

            break;

        case Event::Overlay_Delete:
            m_layer = nullptr;
            break;

        case Event::Overlay_Enable:
            m_viewport->update();
            break;

        case Event::Overlay_Resize:
            break;
    }
}

ViewportLayerPtr OpenGLLayer::layer() const
{
    return m_layer;
}

} // namespace exo
