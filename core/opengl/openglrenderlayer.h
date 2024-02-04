#ifndef EXO_OPENGLRENDERLAYER_H
#define EXO_OPENGLRENDERLAYER_H

#include <QMatrix4x4>
#include <QOffscreenSurface>
#include "opengl/openglviewportlayer.h"
#include "opengl/openglshader.h"
#include "opengl/opengltexture.h"
#include "opengl/openglplane.h"
#include "render/scenerenderer.h"

namespace exo {

class OpenGLRenderLayer : public OpenGLViewportLayer
{
public:
    const static LayerTypeId ViewportLayerId;

    OpenGLRenderLayer();

    LayerCategory category() const override{return LayerCategory_Foreground;}

    void initGL(QOpenGLContext *) override;
    void draw(QOpenGLContext *context) override;

    void viewportSceneDidChange(ScenePtr rootScene, ScenePtr cache) override;
    void viewportSceneWillChange(ScenePtr rootScene, ScenePtr cache) override;
    void viewportWasResized() override;
    void viewportWasZoomed() override;
    void viewportWasOffset() override;
    void viewportWasRotated() override;
protected:
    void receiveEvent(int eventType, void *source, void *data1, void *data2) override;
    void addedToViewport(ViewportPtr viewport) override;

private:
    SceneRenderer m_renderer;
    ScenePtr m_scene;
    OpenGLTexture *m_texture;
    OpenGLShader *m_viewportShader = nullptr;
    OpenGLPlane *m_vektorPlane = nullptr;
    QMatrix4x4 m_cameraMatrix;
    QMatrix4x4 m_viewportMatrix;
    int m_viewCamMatrixLoc;
    int m_viewViewMatrixLoc;
    bool m_isDirty = true;
    bounds_d m_dirtyBounds;

};

} // namespace exo

#endif // EXO_OPENGLRENDERLAYER_H
