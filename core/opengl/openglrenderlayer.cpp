#include "openglrenderlayer.h"
#include "opengl/openglviewport.h"
#include "exocore.h"
#include "stack/scene.h"

namespace exo {
const QByteArray OpenGLRenderLayer::ViewportLayerId = "exo.opengl.layer.render-layer";

OpenGLRenderLayer::OpenGLRenderLayer(): OpenGLViewportLayer(ViewportLayerId)
{
   setIsHidden(true);
}


void OpenGLRenderLayer::initGL(QOpenGLContext *context)
{
   OpenGLViewportLayer::initGL(context);

   m_vektorPlane = new OpenGLPlane(context, bounds_d{-1,-1,1,1}, false);

   m_viewportShader = new OpenGLShader(context, ":/opengl_resources/shader/projectedvertex.vert",
                       ":/opengl_resources/shader/texture/texture_alpha.frag");
   m_viewCamMatrixLoc = m_viewportShader->uniformLocation("mvMatrix");
   m_viewViewMatrixLoc = m_viewportShader->uniformLocation("projMatrix");

   m_texture = new OpenGLTexture(true);
}

void OpenGLRenderLayer::draw(QOpenGLContext *context)
{
   if(!rootScene() || !isEnabled())
       return;



   /*
   OpenGLShader tempShader(context, ":/opengl_resources/shader/BasicTextureVertex.vert",
                       ":/opengl_resources/shader/texture.frag");

   tempShader.bind(context);

   tempShader.setTexture("tex", m_outlineTexture->handle());

*/

   m_viewportShader->bind(context);
   m_viewportShader->setMatrix(m_viewCamMatrixLoc, QMatrix4x4());
   m_viewportMatrix.setToIdentity();
   m_viewportMatrix.ortho(QRect(0,0,openGLViewport()->width(),openGLViewport()->height()));
   m_viewportShader->setMatrix(m_viewViewMatrixLoc, m_viewportMatrix);


   m_vektorPlane->draw(context);

   context->functions()->glUseProgram(0);

}

void OpenGLRenderLayer::viewportSceneDidChange(ScenePtr rootScene, ScenePtr t_scene)
{
   OpenGLViewportLayer::viewportSceneDidChange(rootScene, t_scene);

   if(t_scene)
   {
       t_scene->addEventListener(this, "OpenGLSceneLayer");
       m_renderer.setScene(t_scene);
   }
}

void OpenGLRenderLayer::viewportSceneWillChange(ScenePtr rootScene, ScenePtr t_scene)
{
   OpenGLViewportLayer::viewportSceneWillChange(rootScene, t_scene);

   if(t_scene)
       t_scene->removeEventListener(this);
   m_renderer.setScene(nullptr);
}

void OpenGLRenderLayer::receiveEvent(int eventType, void *source, void *data1, void *data2)
{
   OpenGLViewportLayer::receiveEvent(eventType, source, data1, data2);
   switch (eventType) {
       case Event::Cache_MarkedClean:

           m_isDirty = true;
           openGLViewport()->update();
           break;
   }
}

void OpenGLRenderLayer::viewportWasResized()
{

   size_i size{viewport()->width(), viewport()->height()};
   openGLViewport()->update();

}

void OpenGLRenderLayer::viewportWasZoomed()
{

   openGLViewport()->update();
}

void OpenGLRenderLayer::viewportWasOffset()
{

   openGLViewport()->update();
}

void OpenGLRenderLayer::viewportWasRotated()
{

   openGLViewport()->update();
}

void OpenGLRenderLayer::addedToViewport(ViewportPtr viewport)
{

}


} // namespace exo
