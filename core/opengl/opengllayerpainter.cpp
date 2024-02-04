#include <QImage>
#include <QPainter>
#include "opengllayerpainter.h"
#include "viewport/layer/viewportlayer.h"
//#include "viewport/layer/backgroundlayer.h"
#include "raster/raster.h"

namespace exo {

OpenGLLayerPainter::OpenGLLayerPainter(ViewportLayerPtr layer, OpenGLViewport *viewport):OpenGLLayer(layer, viewport)
{
    if(layer->coordinateSpace() == ViewportLayer::ViewportSpace)
    {
        //m_pixelData = QImage(layer->viewportBounds().width(),layer->viewportBounds().height(),QImage::Format_ARGB32_Premultiplied);

        m_raster = new Raster(viewport->width(),viewport->height(), PixelFormatRGBA8);
        m_raster->setIsPremultiplied(true);
        m_plane = new OpenGLPlane(viewport->context(),bounds_d{-1,-1,1,1},false);
    }
    else {
        bounds_d bnds = layer->bounds();
        if(bnds.is_valid())
        {            
            //m_pixelData = QImage(bnds.width(),bnds.height(),QImage::Format_ARGB32_Premultiplied);
            m_raster = new Raster(bnds.width(),bnds.height(), PixelFormatRGBA8);
            m_raster->setIsPremultiplied(true);
            m_plane = new OpenGLPlane(viewport->context(), bnds);
        }
    }
    m_texture = new OpenGLTexture(true);
    m_texture->resize(viewport->context(), m_raster);
    //if(m_plane)
        //m_plane->create();

    //m_plane->setImage(m_pixelData);
}

OpenGLLayerPainter::~OpenGLLayerPainter()
{
    if(m_plane)
        delete m_plane;

    if(m_raster)
        delete m_raster;
    m_raster = nullptr;
}

void OpenGLLayerPainter::initGL(QOpenGLContext *context)
{
    OpenGLLayer::initGL(context);

    if(layer()->coordinateSpace() == ViewportLayer::SceneSpace)
    {
        m_shader = new OpenGLShader(context, ":/opengl_resources/shader/projectedvertex.vert", ":/opengl_resources/shader/texture.frag");
        m_camMatrixLoc = m_shader->uniformLocation("mvMatrix");
        m_viewMatrixLoc = m_shader->uniformLocation("projMatrix");
    }
    else if(layer()->coordinateSpace() == ViewportLayer::ViewportSpace)
        m_shader = new OpenGLShader(context, ":/opengl_resources/shader/BasicTextureVertex.vert", ":/opengl_resources/shader/texture.frag");
}

void OpenGLLayerPainter::draw(QOpenGLContext *context)
{
    if(!m_shader || !layer()->isRenderable())
        return;

    if(isDirty())
    {
        paintLayer();
    }

    m_shader->bind(context);
    if(layer()->coordinateSpace() == ViewportLayer::SceneSpace)
    {
        m_shader->setMatrix(m_camMatrixLoc, viewport()->cameraMatrix());
        m_shader->setMatrix(m_viewMatrixLoc, viewport()->viewportMatrix());
    }

   // m_texture->bind();
    m_shader->setTexture("tex", m_texture->handle());

    if(m_plane)
        m_plane->draw(context);
    m_shader->clearTextures();
}

void OpenGLLayerPainter::markDirty()
{

    if(layer()->coordinateSpace() == ViewportLayer::SceneSpace)
    {
        //setDirtyBounds(layer()->sceneTransform().mapBounds(m_raster->bounds().translated(-viewport()->sceneOffset())));
        setDirtyBounds(viewport()->controller()->visibleSceneBounds());
    }
    else
        setDirtyBounds(m_raster->bounds());
        /*
    if(layer()->coordinateSpace() == ViewportLayer::SceneSpace)
        setDirtyBounds(layer()->sceneTransform().mapBounds(bounds_i{0,0,m_pixelData.width(), m_pixelData.height()}));
    else
        setDirtyBounds(bounds_i{0,0,m_pixelData.width(), m_pixelData.height()});
        */

    if(m_plane)
    {
        if(viewport()->animateScale() > 4.0)
            m_texture->setMagFilter(GL_NEAREST);
        else
            m_texture->setMagFilter(GL_LINEAR);
    }

}

void OpenGLLayerPainter::paintLayer()
{

    QImage image = m_raster->toQImage();
    QPainter painter(&image);

    bounds_i paintBounds = dirtyBounds();

    //qDebug() << paintBounds;

    painter.setClipRect(QRect(paintBounds));

    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(painter.clipBoundingRect(), layer()->fillColor());
    //paint(&painter,sceneTransform());
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    //rendering_buffer layerBuff(m_pixelData.bits(), static_cast<uint>(m_pixelData.width()), static_cast<uint>(m_pixelData.height()), m_pixelData.bytesPerLine());

    ViewportLayerPtr _layer = layer();
    bounds_i _dirtyBounds = dirtyBounds();

    if(_layer->coordinateSpace() == ViewportLayer::ViewportSpace)
    {
        _layer->paint(&painter,m_raster,_layer->sceneTransform(), _dirtyBounds);
    }
    else if(_layer->coordinateSpace() == ViewportLayer::SceneSpace)
    {
        trans_affine trans;
        trans = _layer->sceneTransform();

        bounds_d convertedDirty{static_cast<double>(_dirtyBounds.x1),
                    static_cast<double>(_dirtyBounds.y1),
                    static_cast<double>(_dirtyBounds.x2),
                    static_cast<double>(_dirtyBounds.y2)};

        //bounds_i b = intersect_bounds(m_dirtyBounds,bounds_i(0,0,m_pixelData.width(), m_pixelData.height()));
        bounds_i b = convertedDirty;

        painter.setClipRect(QRect(b));

        bounds_i lb = _layer->bounds();


        if(lb.intersects(b))
            _layer->paint(&painter,m_raster, trans, intersect_bounds(b, lb));

        //_layer->paint(&painter,layerBuff, trans, lb);


    }
    else
    {
        _layer->paint(&painter,m_raster, trans_affine{}, _dirtyBounds);
    }

    _layer->markClean();
    markClean();


    if(m_plane)
    {
/*

        if(layer()->coordinateSpace() == ViewportLayer::SceneSpace && dirtyBounds().is_valid())
        {
            bounds_i b = intersect_bounds(bounds_i{m_raster->bounds()}, dirtyBounds());
            RasterPtr subRaster = m_raster->subRaster(b);
            m_plane->updateRaster(subRaster, b);
            qDebug() << b << "  " << layer()->layerId();
            delete subRaster;
        }
        else
*/
        m_texture->resize(viewport()->context(), m_raster, m_raster->pixelData());
            //m_plane->setImage(m_pixelData);
        //qDebug() << m_dirtyBounds << " " << m_layer->layerId();
    }

}

void OpenGLLayerPainter::receiveEvent(int eventType, void *source, void *data1, void *data2)
{
    Q_UNUSED(source)
    Q_UNUSED(data2)

    OpenGLLayer::receiveEvent(eventType, source, data1, data2);

    switch (eventType) {
        case Event::Overlay_Enable:
        case Event::Overlay_Resize:
        {

            size_i size{viewport()->width(), viewport()->height()};

            ViewportLayerPtr _layer = layer();

            if(m_raster)
                delete m_raster;
            m_raster = nullptr;

            if(_layer->coordinateSpace() != ViewportLayer::ViewportSpace)
            {
                viewport()->makeCurrent();
                if(m_plane)
                    m_plane->setBounds(layer()->bounds());
                //m_pixelData = QImage(size.width,size.height,QImage::Format_ARGB32_Premultiplied);
                //m_pixelData.fill(Qt::transparent);
                m_raster =new  Raster(_layer->bounds().width(),_layer->bounds().height(), PixelFormatRGBA8);
                m_raster->setIsPremultiplied(true);
                m_texture->resize(viewport()->context(), m_raster);

            }
            else {
                viewport()->makeCurrent();
                m_raster = new Raster(size.width,size.height, PixelFormatRGBA8);
                m_raster->setIsPremultiplied(true);
                m_texture->resize(viewport()->context(), m_raster);
                //m_pixelData = QImage(size.width,size.height,QImage::Format_ARGB32_Premultiplied);
                //m_pixelData.fill(Qt::transparent);
            }
            markDirty();
            viewport()->update();
        }
        break;
    }
}

} // namespace exo
