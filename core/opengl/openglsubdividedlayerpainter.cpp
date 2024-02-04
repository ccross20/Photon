#include <QImage>
#include <QPainter>
#include <QMatrix4x4>
#include <QRandomGenerator>
#include <QtConcurrent>
#include "openglsubdividedlayerpainter.h"
#include "viewport/layer/viewportlayer.h"
//#include "viewport/layer/backgroundlayer.h"
#include "raster/raster.h"

namespace exo {

RenderListener::RenderListener(OpenGLSubdividedLayerPainter *t_painter, QObject *t_parent):QObject(t_parent),m_painter(t_painter)
{
    connect(t_painter->layer(), &ViewportLayer::paintComplete, this, &RenderListener::paintComplete);
}

void RenderListener::paintComplete(int t_id, RasterPtr t_raster)
{
    m_painter->deferredPaintComplete(t_id, t_raster);
}

void OpenGLSubdividedLayerPainter::GridCell::setBounds(const bounds_d &t_bounds)
{
    if(bounds == t_bounds)
        return;

    bounds = t_bounds;
    isDirty = true;
}

OpenGLSubdividedLayerPainter::OpenGLSubdividedLayerPainter(ViewportLayerPtr layer, OpenGLViewport *viewport):OpenGLLayer(layer, viewport),m_listener(this)
{

    int cellWidth = ceil(viewport->width() / (m_columns-1.0));
    int cellHeight = ceil(viewport->height() / (m_rows-1.0));

    for(int i = 0; i < m_columns * m_rows; ++i)
    {
        GridCell cell = GridCell(new OpenGLPlane(viewport->context(),bounds_d{-1,-1,1,1},true, false), new OpenGLTexture(true));
        cell.raster = new Raster(cellWidth, cellHeight, PixelFormatRGBA8);
        cell.index = i;
        cell.raster->setIsPremultiplied(true);
        cell.texture->resize(viewport->context(), PixelFormatRGBA8,cellWidth, cellHeight);
        cell.color = QColor::fromHsvF(QRandomGenerator::global()->generateDouble(),1,1,1);
        m_cells.append(cell);

    }
    auto cellIt = m_cells.begin();
    for(int x = 0; x < m_columns; ++x)
    {
        for(int y = 0; y < m_rows; ++y)
        {
            GridCell &cell = *cellIt;
            cell.originalBounds = bounds_i{x * cellWidth, y * cellHeight, (x+1) * cellWidth, (y+1) * cellHeight};
            ++cellIt;
        }
    }

    //if(m_plane)
        //m_plane->create();

    //m_plane->setImage(m_pixelData);
}

OpenGLSubdividedLayerPainter::~OpenGLSubdividedLayerPainter()
{
    for(auto &cell : m_cells)
    {
        delete cell.raster;
        delete cell.plane;
        delete cell.texture;
    }
}

void OpenGLSubdividedLayerPainter::initGL(QOpenGLContext *context)
{
    OpenGLLayer::initGL(context);

    m_shader = new OpenGLShader(context, ":/opengl_resources/shader/projectedvertex.vert", ":/opengl_resources/shader/texture.frag");
    //m_shader = new OpenGLShader(context, ":/opengl_resources/shader/projectedvertex.vert", ":/opengl_resources/shader/color/fill.frag");
    m_camMatrixLoc = m_shader->uniformLocation("mvMatrix");
    m_viewMatrixLoc = m_shader->uniformLocation("projMatrix");
}

void OpenGLSubdividedLayerPainter::draw(QOpenGLContext *context)
{
    if(!m_shader || !layer()->isRenderable())
        return;

    QMatrix4x4 viewMat;
    viewMat.setToIdentity();
    viewMat.ortho(QRect{0,0,viewport()->width(),viewport()->height()});


    QMatrix4x4 camMat;
    camMat.setToIdentity();

    if(m_offsetIsDirty)
        updateOffset();

    /*
    if(isDirty())
    {
        paintLayer();
    }
    */


    bool painted = false;
    for(auto &cell : m_cells)
    {
        if(cell.paintAvailable)
        {
            paintCell(context,cell);
            painted = true;
        }
    }


    m_shader->bind(context);

    m_shader->setMatrix(m_viewMatrixLoc, viewMat);
    m_shader->setMatrix(m_camMatrixLoc, camMat);

    for(auto &cell : m_cells)
    {
        //if(cell.isDirty)
            //continue;
        //m_shader->setColor("colorFill", cell.color);
        m_shader->setTexture("tex", cell.texture->handle());
        cell.plane->draw(context);
    }

    //if(painted)
    {
        layer()->markClean();
        markClean();
    }


   // m_texture->bind();

    m_shader->clearTextures();
}

void OpenGLSubdividedLayerPainter::resetCellOffset()
{
    m_lastOffset = viewport()->controller()->offset();
    m_cellOffset = point_d{};
    m_offsetIsDirty = true;
    for(auto &cell : m_cells)
        cell.isDirty = true;
}

struct RenderContainer
{
    RenderContainer(RasterPtr t_raster, const trans_affine &t_trans, const bounds_d t_bounds, int t_index):raster(t_raster),
    transform(t_trans),bounds(t_bounds),cellId(t_index){}
    RasterPtr raster;
    trans_affine transform;
    bounds_i bounds;
    int cellId;

};

void OpenGLSubdividedLayerPainter::setDirtyBounds(const bounds_i &bounds)
{
    OpenGLLayer::setDirtyBounds(bounds);
    QList<RenderContainer> renderList;

    for(auto &cell : m_cells)
    {
        if(!cell.bounds.intersects(bounds))
            continue;
        trans_affine trans = viewport()->transform();

        point_d offset = cell.bounds.topLeft();
        offset += m_cellOffset;

        trans.multiply(trans_affine_translation{-offset.x, -offset.y});
        renderList.append(RenderContainer{cell.raster, trans, cell.raster->bounds(),cell.index});
        cell.paintAvailable = true;
        //layer()->requestPaint(cell.raster, trans, cell.raster->bounds(),cell.index);

        ++m_paintCount;
    }

    auto sceneLayer = layer();
    QtConcurrent::blockingMap(renderList, [sceneLayer](const RenderContainer &container){
        container.raster->clear();
        sceneLayer->requestPaint(container.raster, container.transform, container.bounds,container.cellId);

    });

    if(!renderList.isEmpty())
        viewport()->update();


        /*
    if(layer()->coordinateSpace() == ViewportLayer::SceneSpace)
        setDirtyBounds(layer()->sceneTransform().mapBounds(bounds_i{0,0,m_pixelData.width(), m_pixelData.height()}));
    else
        setDirtyBounds(bounds_i{0,0,m_pixelData.width(), m_pixelData.height()});
        */


}

void OpenGLSubdividedLayerPainter::offset(point_d t_offset)
{
    m_cellOffset = (t_offset - m_lastOffset) * viewport()->controller()->scale();

    //qDebug() << t_offset << m_cellOffset;

    m_offsetIsDirty = true;
}

void OpenGLSubdividedLayerPainter::scale(double t_scale)
{
    OpenGLLayer::scale(t_scale);
    resetCellOffset();
}

void OpenGLSubdividedLayerPainter::rotate(double t_rotate)
{
    OpenGLLayer::rotate(t_rotate);
    resetCellOffset();
}

void OpenGLSubdividedLayerPainter::updateOffset()
{
    double cellWidth = ceil(viewport()->width() / (m_columns-1.0));
    double cellHeight = ceil(viewport()->height() / (m_rows-1.0));

    point_d off = m_cellOffset;
    QList<RenderContainer> renderList;

    bounds_d cellBounds{-cellWidth,-cellHeight,cellWidth * (m_columns-1), cellHeight  * (m_rows-1)};

    auto cellIt = m_cells.begin();
    for(int x = 0; x < m_columns; ++x)
    {
        for(int y = 0; y < m_rows; ++y)
        {
            GridCell &cell = *cellIt;
            cell.plane->bind(viewport()->context());

            point_d position{x*cellWidth, y * cellHeight};
            point_d shiftPt;
            position += off;
            if(position.x < cellBounds.left())
                shiftPt.x += cellWidth * m_columns;
            if(position.x > cellBounds.right())
                shiftPt.x -= cellWidth * m_columns;
            if(position.y < cellBounds.top())
                shiftPt.y += cellHeight * m_rows;
            if(position.y > cellBounds.bottom())
                shiftPt.y -= cellHeight * m_rows;

            position += shiftPt;

            cell.setBounds(cell.originalBounds.translated(shiftPt));
            cell.plane->setBounds(bounds_d{position.x, position.y, position.x + cellWidth, position.y + cellHeight});


            trans_affine trans = viewport()->transform();
            cell.raster->setOffset(trans.inverted().mapPoint(position));


            point_d offset = cell.bounds.topLeft();
            offset += m_cellOffset;

            trans.multiply(trans_affine_translation{-offset.x, -offset.y});

            if(cell.isDirty)
            {
                cell.paintAvailable = true;
                renderList.append(RenderContainer{cell.raster, trans, cell.raster->bounds(),cell.index});
                //layer()->requestPaint(cell.raster, trans, cell.raster->bounds(),cell.index);

            }
            //qDebug() << bounds_d{x*cellWidth, y * cellHeight, cellWidth, cellHeight}.translated(off);

            ++cellIt;
        }
    }

    auto sceneLayer = layer();
    QtConcurrent::blockingMap(renderList, [sceneLayer](const RenderContainer &container){
        container.raster->clear();
        sceneLayer->requestPaint(container.raster, container.transform, container.bounds,container.cellId);

    });

    if(!renderList.isEmpty())
        viewport()->update();

    m_offsetIsDirty = false;
}

void OpenGLSubdividedLayerPainter::deferredPaintComplete(int t_id, RasterPtr t_raster)
{
    --m_paintCount;
    if(m_paintCount == 0)
        layer()->endPaintSession();

    if(t_id < 0 || t_id >= m_cells.length())
        return;
    auto &cell = m_cells[t_id];
    cell.paintAvailable = true;
    cell.raster->copyFrom(t_raster);

    if(m_paintCount == 0)
        viewport()->update();
    //qDebug() << "Paint complete" << t_id;
}

void OpenGLSubdividedLayerPainter::resizeCells()
{

    int cellWidth = ceil(viewport()->width() / (m_columns-1.0));
    int cellHeight = ceil(viewport()->height() / (m_rows-1.0));

    for(auto &cell : m_cells)
    {
        delete cell.raster;
        cell.raster = new Raster(cellWidth, cellHeight, PixelFormatRGBA8);
        cell.raster->setIsPremultiplied(true);
        cell.texture->resize(cellWidth, cellHeight);
    }

    auto cellIt = m_cells.begin();
    for(int x = 0; x < m_columns; ++x)
    {
        for(int y = 0; y < m_rows; ++y)
        {
            GridCell &cell = *cellIt;
            cell.originalBounds = bounds_i{x * cellWidth, y * cellHeight, (x+1) * cellWidth, (y+1) * cellHeight};
            ++cellIt;
        }
    }

}

void OpenGLSubdividedLayerPainter::paintCell(QOpenGLContext *t_context, GridCell &t_cell)
{
    //t_cell.raster->fill(t_cell.color);

/*
    t_cell.raster->clear();
    QImage image = t_cell.raster->toQImage();
    QPainter painter(&image);
    //painter.setClipRect(QRect(t_cell.bounds));

    trans_affine trans = viewport()->transform();

    point_d offset = t_cell.bounds.topLeft();
    offset += m_cellOffset;

    trans.multiply(trans_affine_translation{-offset.x, -offset.y});

    painter.setTransform(trans.toQTransform());

    layer()->paint(&painter,t_cell.raster, trans, t_cell.raster->bounds());
    */

    t_cell.isDirty = false;
    t_cell.paintAvailable = false;

    t_cell.texture->bind(t_context);
    t_cell.texture->resize(viewport()->context(), t_cell.raster, t_cell.raster->pixelData());
    t_cell.texture->unbind();

}

void OpenGLSubdividedLayerPainter::receiveEvent(int eventType, void *source, void *data1, void *data2)
{
    Q_UNUSED(source)
    Q_UNUSED(data2)

    OpenGLLayer::receiveEvent(eventType, source, data1, data2);

    switch (eventType) {
        case Event::Overlay_Enable:
        case Event::Overlay_Resize:
        {
            resizeCells();

            offset(point_d{0,0});

            markDirty();
            viewport()->update();
        }
        break;
    }
}

} // namespace exo
