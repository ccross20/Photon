#include <QPainter>
#include <QPainterPath>
#include "drawellipseclipeffect.h"
#include "channel/parameter/numberchannelparameter.h"
#include "channel/parameter/point2channelparameter.h"
#include "channel/parameter/colorchannelparameter.h"
#include "channel/parameter/boolchannelparameter.h"

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
    addChannelParameter(new NumberChannelParameter("strokeWidth", 2.0,0,100));
    addChannelParameter(new ColorChannelParameter("color", Qt::red));
    addChannelParameter(new ColorChannelParameter("strokeColor", Qt::black));
}

void DrawEllipseClipEffect::evaluate(CanvasClipEffectEvaluationContext &t_context) const
{
    QPainter painter{t_context.canvasImage};
    painter.setRenderHint(QPainter::Antialiasing);

    float w = static_cast<float>(t_context.canvasImage->width());
    float h = static_cast<float>(t_context.canvasImage->height());

    QPointF pos = t_context.channelValues["position"].value<QPointF>();
    QPointF center = t_context.channelValues["center"].value<QPointF>();
    QPointF scale = t_context.channelValues["scale"].value<QPointF>();

    QColor color = t_context.channelValues["color"].value<QColor>();
    QColor strokeColor = t_context.channelValues["strokeColor"].value<QColor>();

    double strokeWidth = t_context.channelValues["strokeWidth"].toDouble();


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
