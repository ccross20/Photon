#include <cmath>
#include <QColor>
#include "canvasshapenode.h"
#include "graph/parameter/point2dparameter.h"
#include "graph/parameter/colorparameter.h"

namespace photon {

const QByteArray CanvasShapeNode::Shape = "shape";
const QByteArray CanvasShapeNode::Position = "position";
const QByteArray CanvasShapeNode::Size = "size";
const QByteArray CanvasShapeNode::Rotation = "rotation";
const QByteArray CanvasShapeNode::Roundness = "roundness";
const QByteArray CanvasShapeNode::Sides = "sides";
const QByteArray CanvasShapeNode::Softness = "softness";
const QByteArray CanvasShapeNode::Fill = "fill";
const QByteArray CanvasShapeNode::StrokeColor = "strokeColor";
const QByteArray CanvasShapeNode::StrokeWidth = "strokeWidth";
const QByteArray CanvasShapeNode::Output = "output";

keira::NodeInformation CanvasShapeNode::info()
{
    keira::NodeInformation toReturn([](){return new CanvasShapeNode;});
    toReturn.name = "Shape";
    toReturn.nodeId = "photon.canvas.shape";
    toReturn.categories = {"Canvas"};
    toReturn.graphs = QByteArrayList{"canvas"};

    return toReturn;
}

CanvasShapeNode::CanvasShapeNode() : BaseCanvasNode("photon.canvas.shape")
{
    setName("Shape");
}

void CanvasShapeNode::createParameters()
{
    m_shape = new keira::OptionParameter(Shape, "Shape",
        {"Circle", "Box", "Triangle", "Pentagon", "Hexagon", "Star"}, 0);
    addParameter(m_shape);

    m_position = new Point2DParameter(Position, "Position", QPointF(0.5, 0.5));
    addParameter(m_position);

    m_size = new Point2DParameter(Size, "Size", QPointF(0.3, 0.3));
    addParameter(m_size);

    m_rotation = new keira::DecimalParameter(Rotation, "Rotation", 0.0);
    addParameter(m_rotation);

    // Corner rounding for the box; for the star it controls point sharpness (0 =
    // pointy, 1 = regular polygon).
    m_roundness = new keira::DecimalParameter(Roundness, "Roundness", 0.0);
    m_roundness->setMinimum(0.0);
    addParameter(m_roundness);

    m_sides = new keira::IntegerParameter(Sides, "Sides", 5);
    m_sides->setMinimum(2);
    m_sides->setMaximum(20);
    addParameter(m_sides);

    m_softness = new keira::DecimalParameter(Softness, "Softness", 0.0);
    m_softness->setMinimum(0.0);
    addParameter(m_softness);

    m_fill = new ColorParameter(Fill, "Fill", QColor(255, 255, 255, 255));
    addParameter(m_fill);

    m_strokeColor = new ColorParameter(StrokeColor, "Stroke Color", QColor(0, 0, 0, 255));
    addParameter(m_strokeColor);

    // Stroke width in canvas fraction (0 = no stroke).
    m_strokeWidth = new keira::DecimalParameter(StrokeWidth, "Stroke Width", 0.0);
    m_strokeWidth->setMinimum(0.0);
    addParameter(m_strokeWidth);

    m_outputParam = new RhiTextureParameter(Output, "Canvas", RhiTextureData{}, keira::AllowMultipleOutput);
    addParameter(m_outputParam);
}

void CanvasShapeNode::writeUniforms(QByteArray &out, const QSize &size) const
{
    float *f = reinterpret_cast<float *>(out.data());
    const QPointF pos = m_position->value().toPointF();
    const QPointF sz = m_size->value().toPointF();
    const QColor fill = m_fill->value().value<QColor>();
    const QColor stroke = m_strokeColor->value().value<QColor>();
    const float aspect = size.height() > 0 ? float(size.width()) / float(size.height()) : 1.0f;

    f[0]  = float(pos.x());  f[1]  = float(pos.y());
    f[2]  = float(sz.x());   f[3]  = float(sz.y());
    f[4]  = float(m_rotation->value().toDouble() * M_PI / 180.0);
    f[5]  = float(m_roundness->value().toDouble());
    f[6]  = float(m_softness->value().toDouble());
    f[7]  = aspect;
    f[8]  = float(m_shape->value().toInt());
    f[9]  = float(m_sides->value().toInt());
    f[10] = float(m_strokeWidth->value().toDouble());
    f[11] = 0.0f;
    f[12] = float(fill.redF());   f[13] = float(fill.greenF());
    f[14] = float(fill.blueF());  f[15] = float(fill.alphaF());
    f[16] = float(stroke.redF()); f[17] = float(stroke.greenF());
    f[18] = float(stroke.blueF()); f[19] = float(stroke.alphaF());
}

} // namespace photon
