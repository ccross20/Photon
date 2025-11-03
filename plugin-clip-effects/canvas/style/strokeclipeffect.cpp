#include "strokeclipeffect.h"
#include "channel/parameter/numberchannelparameter.h"
#include "channel/parameter/optionchannelparameter.h"
#include "channel/parameter/colorchannelparameter.h"
#include "channel/parameter/boolchannelparameter.h"
#include "opengl/openglshader.h"
#include "opengl/openglplane.h"
#include "opengl/opengltexture.h"
#include "opengl/distancetransform.h"
#include "photoncore.h"
#include "pixel/canvas.h"

namespace photon {

StrokeClipEffect::StrokeClipEffect() {

}


void StrokeClipEffect::init()
{
    addChannelParameter(new BoolChannelParameter("StrokeOnly"));
    addChannelParameter(new NumberChannelParameter("Thickness"));
    addChannelParameter(new NumberChannelParameter("Offset"));
    addChannelParameter(new NumberChannelParameter("Spacing"));
    addChannelParameter(new NumberChannelParameter("Count"));
    addChannelParameter(new ColorChannelParameter("Color", Qt::red));
    addChannelParameter(new OptionChannelParameter("Mode",{"Inside","Outside"}));

}

void StrokeClipEffect::initializeContext(QOpenGLContext *t_context, Canvas *)
{

}

void StrokeClipEffect::canvasResized(QOpenGLContext *, Canvas *)
{

}

void StrokeClipEffect::evaluate(CanvasEffectEvaluationContext &t_context)
{
    bool strokeOnly = t_context.channelValues["StrokeOnly"].toBool();
    double thickness = t_context.channelValues["Thickness"].toDouble();
    double offset = t_context.channelValues["Offset"].toDouble();
    double spacing = t_context.channelValues["Spacing"].toDouble();
    double count = t_context.channelValues["Count"].toDouble();
    int m = t_context.channelValues["Mode"].toInt();
    QColor color = t_context.channelValues["Color"].value<QColor>();

    QColor alphadColor(Qt::transparent);


    if(spacing <= 0.0)
    {
        thickness = thickness * count;
        count = 1;
    }


    int w = t_context.canvas->width();
    int h = t_context.canvas->height();



#if defined(Q_OS_WIN)
    OpenGLTexture *inputTex;
#elif defined(Q_OS_MAC)
    OpenGLTexture *inputTex(true);
#endif

    inputTex = t_context.buffer->texture();

    OpenGLTexture strokeTex;
    strokeTex.resize(t_context.openglContext, QImage::Format::Format_ARGB32_Premultiplied, w, h);


    OpenGLPlane plane(t_context.openglContext, bounds_d{-1,-1,1,1});
    //OpenGLFrameBuffer frameBuffer(&inputTex, t_context.openglContext);

    DistanceTransform::DistanceMode mode = DistanceTransform::Outer;


    switch (m) {
    case 0: //inside
        mode = DistanceTransform::Inner;
        break;

    case 1: // outside
        mode = DistanceTransform::Outer;
        break;

    case 2: // middle
        mode = DistanceTransform::Middle;
        break;
    }

    auto context = t_context.openglContext;


    DistanceTransform distance(context, inputTex, t_context.buffer, &plane, mode);

    //croppedRaster->clear();
    t_context.buffer->setTexture(&strokeTex);
    context->functions()->glClearColor(0,0,0,0);
    context->functions()->glClear(GL_COLOR_BUFFER_BIT);

    //distance.texture()->bind();

    /*
    OpenGLShader shader(&context, ":/opengl_resources/shader/BasicTextureVertex.vert", ":/opengl_resources/shader/texture.frag");
    shader.bind(&context);
    shader.setTexture("tex", distance.texture()->handle());
    plane.draw();
*/


    OpenGLShader shader(context, ":/resources/shader/BasicTextureVertex.vert", ":/clip-effect-resources/shader/style/stroke.frag");
    shader.bind(context);
    shader.setFloat2("wh_rcp"   , 1.0/w, 1.0/h);
    shader.setColor("colA", alphadColor);
    shader.setColor("colB", color);
    shader.setFloat("thickness", thickness);
    shader.setFloat("offset", offset);
    shader.setFloat("spacing", spacing);
    shader.setInt("count", count);
    shader.setFloat("offset", offset);
    shader.setTexture("tex_dtnn", distance.texture()->handle());
    plane.draw();



    if(!strokeOnly)
    {
        context->functions()->glEnable(GL_BLEND);
        context->functions()->glBlendEquation (GL_FUNC_ADD);


        context->functions()->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);



        t_context.buffer->setTexture(inputTex);
        strokeTex.bind();
        OpenGLShader overlayShader(context, ":/resources/shader/BasicTextureVertex.vert", ":/resources/shader/texture.frag");
        overlayShader.bind(context);

        /*

        switch (m) {
        case 0: //inside
        {
            context->functions()->glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



            t_context.buffer->setTexture(inputTex);
            strokeTex.bind();
            OpenGLShader overlayShader(context, ":/resources/shader/BasicTextureVertex.vert", ":/resources/shader/texture.frag");
            overlayShader.bind(context);
        }
        break;

        case 1: // outside
        {
            context->functions()->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            inputTex->bind();
            OpenGLShader overlayShader(context, ":/resources/shader/BasicTextureVertex.vert", ":/resources/shader/texture.frag");
            overlayShader.bind(context);

        }
        break;
        }
*/


        plane.draw();
    }
    else
    {
        context->functions()->glEnable(GL_BLEND);
        context->functions()->glBlendEquation (GL_FUNC_SUBTRACT);

        context->functions()->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);



        t_context.buffer->setTexture(inputTex);
        strokeTex.bind();
        OpenGLShader overlayShader(context, ":/resources/shader/BasicTextureVertex.vert", ":/resources/shader/texture.frag");
        overlayShader.bind(context);

        /*

        switch (m) {
        case 0: //inside
        {
            context->functions()->glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



            t_context.buffer->setTexture(inputTex);
            strokeTex.bind();
            OpenGLShader overlayShader(context, ":/resources/shader/BasicTextureVertex.vert", ":/resources/shader/texture.frag");
            overlayShader.bind(context);
        }
        break;

        case 1: // outside
        {
            context->functions()->glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            t_context.buffer->setTexture(inputTex);
            strokeTex.bind();
            OpenGLShader overlayShader(context, ":/resources/shader/BasicTextureVertex.vert", ":/resources/shader/texture.frag");
            overlayShader.bind(context);

        }
        break;
        }
*/

        plane.draw();
    }


  /*
#if defined(Q_OS_WIN)
    frameBuffer.writeToRaster(croppedRaster, bounds_i{}, false);
#elif defined(Q_OS_MAC)
    frameBuffer.writeToRaster(croppedRaster, bounds_i{}, true);
#endif
    */


    strokeTex.destroy();


}

ClipEffectInformation StrokeClipEffect::info()
{
    ClipEffectInformation toReturn([](){return new StrokeClipEffect;});
    toReturn.name = "Stroke";
    toReturn.id = "photon.clip.effect.stroke";
    toReturn.categories.append("Style");

    return toReturn;
}

} // namespace photon
