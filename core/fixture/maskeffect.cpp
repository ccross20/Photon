#include "maskeffect_p.h"
#include "sequence/fixtureclip.h"

namespace photon {



MaskEffect::MaskEffect(const QByteArray &t_id):m_impl(new Impl)
{
    m_impl->id = t_id;
    m_impl->uniqueId = QUuid::createUuid().toByteArray();
}

MaskEffect::~MaskEffect()
{
    delete m_impl;
}

int MaskEffect::index() const
{
    return 0;
}

void MaskEffect::setId(const QByteArray &t_id)
{
    m_impl->id = t_id;
}

QByteArray MaskEffect::id() const
{
    return m_impl->id;
}

QByteArray MaskEffect::uniqueId() const
{
    return m_impl->uniqueId;
}

void MaskEffect::setName(const QString &t_name)
{
    m_impl->name = t_name;
}

QString MaskEffect::name() const
{
    return m_impl->name;
}

FixtureClip *MaskEffect::clip() const
{
    return m_impl->clip;
}

void MaskEffect::updated()
{
    if(m_impl->clip)
        m_impl->clip->maskUpdatedSlot(this);
}

MaskEffect *MaskEffect::previousEffect() const
{
    return m_impl->previousEffect;
}

QVector<Fixture*> MaskEffect::process(const QVector<Fixture*> fixtures) const
{
    return m_impl->fixtures;
}

void MaskEffect::restore(Project &)
{

}

void MaskEffect::readFromJson(const QJsonObject &t_json)
{
    if(t_json.contains("name"))
        m_impl->name = t_json.value("name").toString();
    m_impl->uniqueId = t_json.value("uniqueId").toString(QUuid::createUuid().toString()).toLatin1();
}

void MaskEffect::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("name", m_impl->name);
    t_json.insert("uniqueId", QString(m_impl->uniqueId));
}

} // namespace photon
