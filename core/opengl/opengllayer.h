#ifndef DECO_OPENGLLAYER_H
#define DECO_OPENGLLAYER_H


namespace photon {

class OpenGLLayer
{
public:
    OpenGLLayer();
    virtual ~OpenGLLayer();


    virtual void initGL(QOpenGLContext *){}
    virtual void draw(QOpenGLContext *){}
    virtual void receiveEvent(int eventType, void *source, void *data1, void *data2) override;
    ViewportLayerPtr layer() const;
    virtual void markDirty();
    virtual void offset(point_d t_offset);
    virtual void scale(double t_scale);
    virtual void rotate(double t_rotate);
    virtual void markClean();
    virtual void setDirtyBounds(const bounds_i &bounds);
    bounds_i dirtyBounds() const;
    OpenGLViewport *viewport() const;
    bool isDirty() const;

private:
    bounds_i m_dirtyBounds;
    bool m_isDirty = true;
};

} // namespace exo

#endif // DECO_OPENGLLAYER_H
