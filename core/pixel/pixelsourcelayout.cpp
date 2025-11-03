#include "pixelsourcelayout.h"
#include "pixelsource.h"
#include "util/utils.h"
#include "scene/sceneiterator.h"
#include "project/project.h"
#include "fixture/fixture.h"
#include "fixture/capability/colorcapability.h"
#include "fixture/capability/fixturecapability.h"
#include "pixel/fixturepixelsource.h"

namespace photon {

class PixelSourceLayout::Impl
{
public:
    void rebuildBounds();

    QPointF position;
    QPointF scale = QPointF{1,1};
    QTransform transform;
    bounds_d bounds;
    double rotation = 0;
    QString name;
    QByteArray uniqueId;
    PixelSource *source = nullptr;
};

void PixelSourceLayout::Impl::rebuildBounds()
{
    if(!source)
        return;

    transform = QTransform{};
    transform.translate(position.x(), position.y());
    transform.scale(scale.x(), scale.y());
    transform.rotate(rotation);



    bounds_d bnds;

    for(const auto &pt : source->positions())
        bnds.unite(QPointF{pt.x() * scale.x(), pt.y() * scale.y()});

    bounds = bnds;
}

PixelSourceLayout::PixelSourceLayout(PixelSource *t_source) : QObject(), m_impl(new Impl)
{
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
    m_impl->source = t_source;

    m_impl->rebuildBounds();
}

PixelSourceLayout::~PixelSourceLayout()
{
    delete m_impl;
}

void PixelSourceLayout::setPosition(const QPointF &t_value)
{
    m_impl->position = t_value;
    m_impl->rebuildBounds();
    emit transformUpdated();
}

QPointF PixelSourceLayout::position() const
{
    return m_impl->position;
}


void PixelSourceLayout::setRotation(double t_value)
{
    m_impl->rotation = t_value;
    m_impl->rebuildBounds();
    emit transformUpdated();
}

double PixelSourceLayout::rotation() const
{
    return m_impl->rotation;
}


void PixelSourceLayout::setScale(const QPointF &t_scale)
{
    m_impl->scale = t_scale;
    m_impl->rebuildBounds();
    emit transformUpdated();
}

QPointF PixelSourceLayout::scale() const
{
    return m_impl->scale;
}


QTransform PixelSourceLayout::transform() const
{
    return m_impl->transform;
}


bounds_d PixelSourceLayout::canvasBounds() const
{
    return m_impl->bounds;
}


bounds_d PixelSourceLayout::localBounds() const
{
    return m_impl->bounds;
}

PixelSource *PixelSourceLayout::source() const
{
    return m_impl->source;
}

void PixelSourceLayout::setSource(PixelSource *t_source)
{
    m_impl->source = t_source;
    m_impl->rebuildBounds();
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
        if(!m_impl->source)
        {
            auto fixture = dynamic_cast<Fixture*>(sourceObj);

            if(fixture)
            {
                if(!fixture->findCapability(Capability_Color).isEmpty()){
                    auto capabilities = fixture->findCapability(Capability_Color);
                    m_impl->source = new FixturePixelSource(capabilities);
                }
            }
        }
        m_impl->rebuildBounds();
    }
    else
    {
        qDebug() << "could not find source:" << m_impl->name;
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
