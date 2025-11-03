#include "surfacegizmo_p.h"

namespace photon {

SurfaceGizmo::Impl::Impl(SurfaceGizmo *t_facade):facade(t_facade)
{

}

SurfaceGizmo::SurfaceGizmo(QByteArray t_type, QObject *parent)
    : QObject{parent}, m_impl(new Impl(this))
{
    m_impl->type = t_type;
    m_impl->uniqueId = QUuid::createUuid().toString().toLatin1();
}

SurfaceGizmo::~SurfaceGizmo()
{
    delete m_impl;
}


SurfaceGizmoWidget *SurfaceGizmo::createWidget(SurfaceMode mode)
{
    return nullptr;
}

QByteArray SurfaceGizmo::type() const
{
    return m_impl->type;
}

void SurfaceGizmo::setType(const QByteArray t_type)
{
    m_impl->type = t_type;
}

QByteArray SurfaceGizmo::uniqueId() const
{
    return m_impl->uniqueId;
}

void SurfaceGizmo::setId(const QByteArray &t_value)
{
    m_impl->id = t_value;
    markChanged();
}

QByteArray SurfaceGizmo::id() const
{
    return m_impl->id;
}

void SurfaceGizmo::markChanged()
{
    emit gizmoUpdated();
}

void SurfaceGizmo::processChannels(ProcessContext &t_context)
{

}

void SurfaceGizmo::restore(Project &t_project)
{

}

void SurfaceGizmo::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{

    m_impl->type = t_json.value("type").toString().toLatin1();
    m_impl->uniqueId = t_json.value("uniqueId").toString().toLatin1();
    m_impl->name = t_json.value("name").toString();
    m_impl->id = t_json.value("id").toString().toLatin1();

    if(m_impl->uniqueId.isEmpty())
        m_impl->uniqueId = QUuid::createUuid().toString().toLatin1();

    qDebug() << "Load " << m_impl->uniqueId;
}

void SurfaceGizmo::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("type", QString(m_impl->type));
    t_json.insert("uniqueId", QString(m_impl->uniqueId));
    t_json.insert("id", QString(m_impl->id));
    t_json.insert("name", QString(m_impl->name));
}


} // namespace photon
