#include <QPainter>
#include <QPainterPath>
#include <QMatrix4x4>
#include "drawellipseclipeffect.h"
#include "channel/parameter/numberchannelparameter.h"
#include "channel/parameter/point2channelparameter.h"
#include "channel/parameter/colorchannelparameter.h"
#include "channel/parameter/boolchannelparameter.h"
#include "opengl/openglshader.h"
#include "opengl/openglplane.h"
#include "opengl/opengltexture.h"
#include "photoncore.h"

namespace photon {

DrawEllipseClipEffect::DrawEllipseClipEffect()
{

}

void DrawEllipseClipEffect::init()
{
    addChannelParameter(new Point2ChannelParameter("position",QPointF{.5,.5}));
    addChannelParameter(new Point2ChannelParameter("center",QPointF{.5,.5}));
    addChannelParameter(new BoolChannelParameter("circle"));
    addChannelParameter(new Point2ChannelParameter("scale",QPointF{.5,.5}));
    addChannelParameter(new NumberChannelParameter("rotation"));
    addChannelParameter(new NumberChannelParameter("falloff"));
    addChannelParameter(new NumberChannelParameter("strokeWidth", 2.0,0,100));
    addChannelParameter(new ColorChannelParameter("color", Qt::red));
    addChannelParameter(new ColorChannelParameter("strokeColor", Qt::black));


}

void DrawEllipseClipEffect::initializeContext(QOpenGLContext *t_context, Canvas *)
{
    m_plane = new OpenGLPlane(t_context, bounds_d{-1,-1,1,1}, false);
    m_shader = new OpenGLShader(t_context, ":/resources/shader/projectedvertex.vert",
                                ":/clip-effect-resources/shader/ellipse.frag");
    m_shader->bind(t_context);
    m_viewportLoc = m_shader->uniformLocation("projMatrix");
    m_cameraLoc = m_shader->uniformLocation("mvMatrix");

    m_circleShader = new OpenGLShader(t_context, ":/resources/shader/projectedvertex.vert",
                                      ":/clip-effect-resources/shader/circle.frag");
    m_circleShader->bind(t_context);
    m_circleViewportLoc = m_circleShader->uniformLocation("projMatrix");
    m_circleCameraLoc = m_circleShader->uniformLocation("mvMatrix");
}

void DrawEllipseClipEffect::canvasResized(QOpenGLContext *, Canvas *)
{

}

void DrawEllipseClipEffect::evaluate(CanvasEffectEvaluationContext &t_context)
{
    //float w = static_cast<float>(t_context.canvasImage->width());
    //float h = static_cast<float>(t_context.canvasImage->height());


    QPointF pos = t_context.channelValues["position"].value<QPointF>();
    QPointF center = t_context.channelValues["center"].value<QPointF>();
    QPointF scale = t_context.channelValues["scale"].value<QPointF>();

    QColor color = t_context.channelValues["color"].value<QColor>();
    QColor strokeColor = t_context.channelValues["strokeColor"].value<QColor>();

    double strokeWidth = t_context.channelValues["strokeWidth"].toDouble();
    double rotation = t_context.channelValues["rotation"].toDouble();
    double falloff = t_context.channelValues["falloff"].toDouble();


    bool isCircle = scale.x() == scale.y();
    OpenGLShader *shader = nullptr;
    if(isCircle)
        shader = m_circleShader;
    else
        shader = m_shader;

    shader->bind(t_context.openglContext);


    QMatrix4x4 mat;
    mat.setToIdentity();
    mat.ortho(-1,1, -1,1,-1,1);

    shader->setMatrix(isCircle ? m_circleViewportLoc : m_viewportLoc,mat);

    QMatrix4x4 camMatrix;
    camMatrix.setToIdentity();
    camMatrix.translate(pos.x()*2.0, pos.y()*2.0);
    camMatrix.translate(center.x()*2.0, center.y()*2.0);
    camMatrix.rotate(rotation,0,0,1);
    camMatrix.translate(-center.x()*2.0, -center.y()*2.0);
    shader->setMatrix(isCircle ? m_circleCameraLoc : m_cameraLoc, camMatrix);

    //m_canvas->texture()->bind(t_context.openglContext);
    //m_shader->setTexture("tex",m_canvas->texture()->handle());

    if(strokeWidth > 0)
    {
        shader->setColor("fillColor", strokeColor);
        shader->setFloat2("ratio", scale.x() + (strokeWidth/100.0), scale.y()+ (strokeWidth/100.0));
        shader->setFloat("falloff", 0);
        m_plane->draw(t_context.openglContext);
    }




    shader->setColor("fillColor", color);
    shader->setFloat2("ratio", scale.x(), scale.y());
    shader->setFloat("falloff", falloff);


    m_plane->draw(t_context.openglContext);
    shader->unbind();
    /*
    QPainter painter{t_context.canvasImage};
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setOpacity(t_context.strength);




    QPointF relScale{scale.x() * w,scale.y() * h};

    if(t_context.channelValues["circle"].toBool())
    {
        relScale.setY(relScale.x());
    }


    QPointF relPos{pos.x() * w,pos.y() * h};
    QPointF relCenter{center.x() * relScale.x(),center.y() * relScale.y()};

    //painter.translate(-relCenter);
    QTransform trans;
    //trans.translate(relCenter.x(), relCenter.y());
    trans.translate(relPos.x(), relPos.y());
    trans.rotate(t_context.channelValues["rotation"].toDouble());
    //trans.translate(relCenter.x(), relCenter.y());
    //painter.translate(relCenter);


    if(strokeWidth <= 0.0)
        painter.setPen(Qt::NoPen);
    else
        painter.setPen(QPen(strokeColor, strokeWidth));

    QPainterPath path;
    path.addEllipse(-relCenter.x(), -relCenter.y(),relScale.x(),relScale.y());

    //painter.setOpacity(m_alphaParam->value().toDouble());
    //painter.fillPath(m_pathInputParam->value().value<QPainterPath>(),m_colorParam->value().value<QColor>());
    painter.setBrush(color);
    painter.drawPath(trans.map(path));
*/
}

ClipEffectInformation DrawEllipseClipEffect::info()
{
    ClipEffectInformation toReturn([](){return new DrawEllipseClipEffect;});
    toReturn.name = "Draw Ellipse";
    toReturn.id = "photon.clip.effect.draw-ellipse";
    toReturn.categories.append("Draw");

    return toReturn;
}

} // namespace photon
