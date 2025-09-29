#include "falloffeffect_p.h"
#include "sequence/fixtureclip.h"

namespace photon {

FalloffEffect::FalloffEffect(const QByteArray &t_id):m_impl(new Impl)
{
    m_impl->id = t_id;
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
}

FalloffEffect::~FalloffEffect()
{
    delete m_impl;
}

int FalloffEffect::index() const
{
    return 0;
}

QObject *FalloffEffect::parent() const
{
    return m_impl->parent;
}

void FalloffEffect::setParent(QObject *t_parent)
{
    m_impl->parent = t_parent;
}

void FalloffEffect::setId(const QByteArray &t_id)
{
    m_impl->id = t_id;
}

QByteArray FalloffEffect::id() const
{
    return m_impl->id;
}

QByteArray FalloffEffect::uniqueId() const
{
    return m_impl->uniqueId;
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
    if(m_impl->parent)
    {
        FixtureClip *clip = dynamic_cast<FixtureClip*>(m_impl->parent);
        if(clip)
            clip->falloffUpdatedSlot(this);
    }
}

FalloffEffect *FalloffEffect::previousEffect() const
{
    return m_impl->previousEffect;
}

void FalloffEffect::readFromJson(const QJsonObject &t_json)
{
    if(t_json.contains("name"))
        m_impl->name = t_json.value("name").toString();
    m_impl->uniqueId = t_json.value("uniqueId").toString(QUuid::createUuid().toString()).toLatin1();

}

void FalloffEffect::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("uniqueId", QString(m_impl->uniqueId));
}

} // namespace photon
