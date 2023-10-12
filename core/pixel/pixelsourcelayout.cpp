#include "pixelsourcelayout.h"
#include "pixelsource.h"
#include "util/utils.h"
#include "scene/sceneiterator.h"
#include "project/project.h"

namespace photon {

class PixelSourceLayout::Impl
{
public:
    QPointF position;
    QPointF scale;
    double rotation;
    QString name;
    QByteArray uniqueId;
    PixelSource *source = nullptr;
};

PixelSourceLayout::PixelSourceLayout(PixelSource *t_source) : QObject(), m_impl(new Impl)
{
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
    m_impl->source = t_source;
}

PixelSourceLayout::~PixelSourceLayout()
{

}

void PixelSourceLayout::setPosition(const QPointF &)
{
    delete m_impl;
}

QPointF PixelSourceLayout::position() const
{
    return m_impl->position;
}


void PixelSourceLayout::setRotation(double)
{

}

double PixelSourceLayout::rotation() const
{
    return m_impl->rotation;
}


void PixelSourceLayout::setScale(const QPointF &)
{

}

QPointF PixelSourceLayout::scale() const
{
    return m_impl->scale;
}


QTransform PixelSourceLayout::transform() const
{
    QTransform tform;

    return tform;
}


QRectF PixelSourceLayout::canvasBounds() const
{
    return QRectF{};
}


QRectF PixelSourceLayout::localBounds() const
{
    return QRectF{};
}

PixelSource *PixelSourceLayout::source() const
{
    return m_impl->source;
}

void PixelSourceLayout::setSource(PixelSource *t_source)
{
    m_impl->source = t_source;
}

void PixelSourceLayout::process(ProcessContext &t_context) const
{
    if(m_impl->source)
        m_impl->source->process(t_context, transform());
}

void PixelSourceLayout::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    m_impl->position = jsonToPointF(t_json.value("position").toObject());
    m_impl->scale = jsonToPointF(t_json.value("scale").toObject());
    m_impl->rotation = t_json.value("rotation").toDouble();
    m_impl->name = t_json.value("name").toString();
    m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();

    QByteArray sourceId = t_json.value("source").toString().toLatin1();

    auto sourceObj = SceneIterator::FindOne(t_context.project->sceneRoot(),[sourceId](SceneObject *obj){

        return obj->uniqueId() == sourceId;
    });

    if(sourceObj)
    {
        m_impl->source = dynamic_cast<PixelSource*>(sourceObj);
    }

}

void PixelSourceLayout::writeToJson(QJsonObject &t_json) const
{


    t_json.insert("position", pointFToJson(m_impl->position));
    t_json.insert("scale", pointFToJson(m_impl->scale));
    t_json.insert("rotation", m_impl->rotation);
    t_json.insert("name", m_impl->name);
    t_json.insert("uniqueId", QString(m_impl->uniqueId));

    if(m_impl->source)
        t_json.insert("source", QString(m_impl->source->sourceUniqueId()));
    else
        t_json.insert("source", "");


}

} // namespace photon
