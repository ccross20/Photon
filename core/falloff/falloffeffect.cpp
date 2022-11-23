#include "falloffeffect_p.h"
#include "sequence/clip.h"

namespace photon {

FalloffEffect::FalloffEffect(const QByteArray &t_id):m_impl(new Impl)
{
    m_impl->id = t_id;
}

FalloffEffect::~FalloffEffect()
{
    delete m_impl;
}

Clip *FalloffEffect::clip() const
{
    return m_impl->clip;
}

int FalloffEffect::index() const
{
    return 0;
}

void FalloffEffect::setId(const QByteArray &t_id)
{
    m_impl->id = t_id;
}

QByteArray FalloffEffect::id() const
{
    return m_impl->id;
}

void FalloffEffect::setName(const QString &t_name)
{
    m_impl->name = t_name;
}

QString FalloffEffect::name() const
{
    return m_impl->name;
}

void FalloffEffect::updated()
{
    if(m_impl->clip)
        m_impl->clip->falloffUpdatedSlot(this);
}

FalloffEffect *FalloffEffect::previousEffect() const
{
    return m_impl->previousEffect;
}

void FalloffEffect::readFromJson(const QJsonObject &t_json)
{
    if(t_json.contains("name"))
        m_impl->name = t_json.value("name").toString();
}

void FalloffEffect::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
}

} // namespace photon
