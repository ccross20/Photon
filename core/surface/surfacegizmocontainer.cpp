#include <QJsonArray>
#include "surfacegizmocontainer.h"
#include "surfacegizmo.h"
#include "gizmofactory.h"

namespace photon {

class SurfaceGizmoContainer::Impl
{
public:
    QVector<SurfaceGizmo*> gizmos;
};

SurfaceGizmoContainer::SurfaceGizmoContainer(QObject *parent):QObject(parent),m_impl(new Impl)
{
}

SurfaceGizmoContainer::~SurfaceGizmoContainer()
{
    delete m_impl;
}

void SurfaceGizmoContainer::processChannels(ProcessContext &, double)
{
}

const QVector<SurfaceGizmo*> &SurfaceGizmoContainer::gizmos() const
{
    return m_impl->gizmos;
}

QVector<SurfaceGizmo*> SurfaceGizmoContainer::gizmosByType(const QByteArray &t_type) const
{
    QVector<SurfaceGizmo*> results;
    for(auto gizmo : m_impl->gizmos)
    {
        if(gizmo->type() == t_type)
            results.append(gizmo);
    }
    return results;
}

SurfaceGizmo *SurfaceGizmoContainer::findGizmoWithId(const QByteArray &t_id) const
{
    if(t_id.isEmpty())
        return nullptr;
    for(auto gizmo : m_impl->gizmos)
    {
        if(gizmo->id() == t_id)
            return gizmo;
    }
    return nullptr;
}

SurfaceGizmo *SurfaceGizmoContainer::findGizmoWithUniqueId(const QByteArray &t_id) const
{
    if(t_id.isEmpty())
        return nullptr;
    for(auto gizmo : m_impl->gizmos)
    {
        if(gizmo->uniqueId() == t_id)
            return gizmo;
    }
    return nullptr;
}

void SurfaceGizmoContainer::addGizmo(photon::SurfaceGizmo *t_gizmo)
{
    if(m_impl->gizmos.contains(t_gizmo))
        return;
    emit gizmoWillBeAdded(t_gizmo, m_impl->gizmos.length());
    m_impl->gizmos.append(t_gizmo);
    t_gizmo->setId("Gizmo " + QByteArray::number(m_impl->gizmos.length()));
    t_gizmo->setParent(this);
    emit gizmoWasAdded(t_gizmo, m_impl->gizmos.length()-1);
}

void SurfaceGizmoContainer::removeGizmo(photon::SurfaceGizmo *t_gizmo)
{
    if(!m_impl->gizmos.contains(t_gizmo))
        return;

    int index = m_impl->gizmos.indexOf(t_gizmo);
    emit gizmoWillBeRemoved(t_gizmo, index);
    m_impl->gizmos.removeOne(t_gizmo);
    emit gizmoWasRemoved(t_gizmo, index);
}

void SurfaceGizmoContainer::restore(Project &t_project)
{
    for(auto source : std::as_const(m_impl->gizmos))
        source->restore(t_project);
}

void SurfaceGizmoContainer::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    if(t_json.contains("gizmos"))
    {
        m_impl->gizmos.clear();
        auto sourceArray = t_json.value("gizmos").toArray();
        for(const auto source : std::as_const(sourceArray))
        {
            auto sourceObj = source.toObject();
            auto newGizmo = GizmoFactory::createGizmo(sourceObj.value("type").toString().toLatin1());
            if(newGizmo)
            {
                newGizmo->readFromJson(sourceObj, t_context);
                m_impl->gizmos.append(newGizmo);
                newGizmo->setParent(this);
            }
        }
    }
}

void SurfaceGizmoContainer::writeToJson(QJsonObject &t_json) const
{
    QJsonArray gizmoArray;
    for(auto source : std::as_const(m_impl->gizmos))
    {
        QJsonObject sourceObj;
        source->writeToJson(sourceObj);
        gizmoArray.append(sourceObj);
    }
    t_json.insert("gizmos", gizmoArray);
}

} // namespace photon
