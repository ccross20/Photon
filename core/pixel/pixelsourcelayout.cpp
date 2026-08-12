#include <QJsonArray>
#include "pixelsourcelayout.h"
#include "pixelsource.h"
#include "pixelarrange.h"
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
    void ensurePixelPositions();

    QString name;
    QByteArray uniqueId;
    PixelSource *source = nullptr;
    QVector<QPointF> pixelPositions;
};

// Reconciles pixelPositions against source->pixelCount(): no-op if already
// matching, seeds only newly-appended tail slots (via PixelArrange::linear())
// if growing, truncates if shrinking. Existing entries - including hand-
// dragged ones - are never touched.
void PixelSourceLayout::Impl::ensurePixelPositions()
{
    if(!source)
        return;

    int count = source->pixelCount();
    if(pixelPositions.size() == count)
        return;

    if(pixelPositions.size() > count)
    {
        pixelPositions.resize(count);
        return;
    }

    int existing = pixelPositions.size();
    QVector<QPointF> seeded = PixelArrange::linear(count);
    pixelPositions.resize(count);
    for(int i = existing; i < count; ++i)
        pixelPositions[i] = seeded[i];
}

PixelSourceLayout::PixelSourceLayout(PixelSource *t_source) : QObject(), m_impl(new Impl)
{
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
    m_impl->source = t_source;
}

PixelSourceLayout::~PixelSourceLayout()
{
    delete m_impl;
}

PixelSource *PixelSourceLayout::source() const
{
    return m_impl->source;
}

void PixelSourceLayout::setSource(PixelSource *t_source)
{
    m_impl->source = t_source;
}

int PixelSourceLayout::pixelCount() const
{
    m_impl->ensurePixelPositions();
    return m_impl->pixelPositions.size();
}

const QVector<QPointF> &PixelSourceLayout::pixelPositions() const
{
    m_impl->ensurePixelPositions();
    return m_impl->pixelPositions;
}

QPointF PixelSourceLayout::pixelPosition(int t_index) const
{
    m_impl->ensurePixelPositions();
    if(t_index < 0 || t_index >= m_impl->pixelPositions.size())
        return QPointF();
    return m_impl->pixelPositions[t_index];
}

void PixelSourceLayout::setPixelPosition(int t_index, const QPointF &t_value)
{
    m_impl->ensurePixelPositions();
    if(t_index < 0 || t_index >= m_impl->pixelPositions.size())
        return;

    QRectF bounds = pixelBounds();
    QPointF clamped(qBound(bounds.left(), t_value.x(), bounds.right()),
                     qBound(bounds.top(), t_value.y(), bounds.bottom()));

    m_impl->pixelPositions[t_index] = clamped;
    emit pixelPositionsChanged();
}

QRectF PixelSourceLayout::pixelBounds()
{
    return QRectF(0, 0, 1, 1);
}

void PixelSourceLayout::collectSampleUVs(QVector<QPointF> &t_out) const
{
    if(m_impl->source)
        m_impl->source->collectSampleUVs(t_out, pixelPositions());
}

void PixelSourceLayout::process(ProcessContext &t_context, double t_blend) const
{
    if(m_impl->source)
        m_impl->source->process(t_context, pixelPositions(), t_blend);
}

void PixelSourceLayout::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    m_impl->name = t_json.value("name").toString();
    m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();

    m_impl->pixelPositions.clear();
    for(const auto &v : t_json.value("pixelPositions").toArray())
        m_impl->pixelPositions << jsonToPointF(v.toObject());

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
    }
    else
    {
        qDebug() << "could not find source:" << m_impl->name;
    }

}

void PixelSourceLayout::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("uniqueId", QString(m_impl->uniqueId));

    if(m_impl->source)
        t_json.insert("source", QString(m_impl->source->sourceUniqueId()));
    else
        t_json.insert("source", "");

    m_impl->ensurePixelPositions();
    QJsonArray posArray;
    for(const auto &pt : m_impl->pixelPositions)
        posArray.append(pointFToJson(pt));
    t_json.insert("pixelPositions", posArray);
}

} // namespace photon
