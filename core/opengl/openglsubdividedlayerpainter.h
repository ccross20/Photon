#ifndef EXO_OPENGLSUBDIVIDEDLAYERPAINTER_H
#define EXO_OPENGLSUBDIVIDEDLAYERPAINTER_H

#include <QObject>
#include <QPainter>
#include "openglshader.h"
#include "opengltexture.h"
#include "openglplane.h"
#include "event/ieventlistener.h"
#include "opengllayer.h"
#include "raster/raster.h"

namespace exo {

class OpenGLSubdividedLayerPainter;

class RenderListener : public QObject
{
public:

    RenderListener(OpenGLSubdividedLayerPainter *t_painter, QObject *t_parent = nullptr);

public slots:
    void paintComplete(int t_id, RasterPtr t_raster);

private:
    OpenGLSubdividedLayerPainter *m_painter;
};


class OpenGLSubdividedLayerPainter : public OpenGLLayer
{
public:
    struct GridCell
    {
        GridCell(OpenGLPlane *t_plane, OpenGLTexture *t_tex):plane(t_plane),texture(t_tex){}
        void setBounds(const bounds_d &t_bounds);
        OpenGLPlane *plane;
        OpenGLTexture *texture;
        RasterPtr raster;
        Color color;
        bounds_d originalBounds;
        bounds_d bounds;
        int index;
        bool paintRequested = false;
        bool paintAvailable = false;
        bool isDirty = true;
    };

    OpenGLSubdividedLayerPainter(ViewportLayerPtr layer, OpenGLViewport *viewport);
    ~OpenGLSubdividedLayerPainter() override;


    void initGL(QOpenGLContext *) override;
    void draw(QOpenGLContext *) override;
    void receiveEvent(int eventType, void *source, void *data1, void *data2) override;
    void setDirtyBounds(const bounds_i &bounds) override;
    void offset(point_d t_offset) override;
    void scale(double t_scale) override;
    void rotate(double t_rotate) override;
    void deferredPaintComplete(int t_id, RasterPtr t_raster);

private:
    void resetCellOffset();
    void updateOffset();
    void resizeCells();
    void paintCell(QOpenGLContext *, GridCell &t_cell);

    int m_camMatrixLoc;
    int m_viewMatrixLoc;
    OpenGLShader *m_shader;
    QVector<GridCell> m_cells;
    int m_rows = 6;
    int m_columns = 6;
    point_d m_cellOffset;
    point_d m_lastOffset;
    RenderListener m_listener;
    //QImage m_pixelData;
    int m_paintCount = 0;
    bool m_offsetIsDirty = true;
};

} // namespace exo

#endif // EXO_OPENGLSUBDIVIDEDLAYERPAINTER_H
