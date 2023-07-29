#include <QPainter>
#include <QPainterPath>
#include "drawrectangleclipeffect.h"
#include "channel/parameter/numberchannelparameter.h"
#include "channel/parameter/point2channelparameter.h"

namespace photon {

DrawRectangleClipEffect::DrawRectangleClipEffect()
{

}

void DrawRectangleClipEffect::init()
{
    addChannelParameter(new Point2ChannelParameter("position",QPointF{.5,.5}));
    addChannelParameter(new Point2ChannelParameter("center",QPointF{.5,.5}));
    addChannelParameter(new Point2ChannelParameter("scale",QPointF{.5,.5}));
    addChannelParameter(new NumberChannelParameter("rotation"));
    addChannelParameter(new NumberChannelParameter("strokeWidth", 2.0));
}

void DrawRectangleClipEffect::evaluate(CanvasClipEffectEvaluationContext &t_context) const
{
    QPainter painter{t_context.canvasImage};
    painter.setRenderHint(QPainter::Antialiasing);

    float w = static_cast<float>(t_context.canvasImage->width());
    float h = static_cast<float>(t_context.canvasImage->height());

    QPointF pos = t_context.channelValues["position"].value<QPointF>();
    QPointF center = t_context.channelValues["center"].value<QPointF>();
    QPointF scale = t_context.channelValues["scale"].value<QPointF>();


    QPointF relScale{scale.x() * w,scale.y() * h};
    QPointF relPos{pos.x() * w,pos.y() * h};
    QPointF relCenter{center.x() * relScale.x(),center.y() * relScale.y()};

    //painter.translate(-relCenter);
    QTransform trans;
    //trans.translate(relCenter.x(), relCenter.y());
    trans.translate(relPos.x(), relPos.y());
    trans.rotate(t_context.channelValues["rotation"].toDouble());
    //trans.translate(relCenter.x(), relCenter.y());
    //painter.translate(relCenter);

    painter.setPen(Qt::NoPen);

    QPainterPath path;
    path.addRect(-relCenter.x(), -relCenter.y(),relScale.x(),relScale.y());

    //painter.setOpacity(m_alphaParam->value().toDouble());
    //painter.fillPath(m_pathInputParam->value().value<QPainterPath>(),m_colorParam->value().value<QColor>());
    painter.fillPath(trans.map(path), Qt::red);

}

ClipEffectInformation DrawRectangleClipEffect::info()
{
    ClipEffectInformation toReturn([](){return new DrawRectangleClipEffect;});
    toReturn.name = "Draw Rectangle";
    toReturn.id = "photon.clip.effect.draw-rectangle";
    toReturn.categories.append("Draw");

    return toReturn;
}

void DrawRectangleClipEffect::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    ClipEffect::readFromJson(t_json, t_context);
}

void DrawRectangleClipEffect::writeToJson(QJsonObject &t_json) const
{
    ClipEffect::writeToJson(t_json);
}


} // namespace photon
