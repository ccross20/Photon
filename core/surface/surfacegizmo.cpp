#include <QJsonArray>
#include "surfacegizmo_p.h"

namespace photon {

const QByteArray SurfaceGizmo::GizmoPressId = "GizmoPress";
const QByteArray SurfaceGizmo::GizmoReleaseId = "GizmoRelease";

SurfaceGizmo::Impl::Impl(SurfaceGizmo *t_facade):facade(t_facade)
{

}

SurfaceGizmo::SurfaceGizmo(QByteArray t_type, QObject *parent)
    : QObject{parent}, m_impl(new Impl(this))
{
    m_impl->type = t_type;
    m_impl->uniqueId = QUuid::createUuid().toString().toLatin1();

    // A user-facing identifier, common to every gizmo. Empty by default; once
    // set, it's what shows up wherever a gizmo needs to be picked from a list
    // (e.g. the Gizmo Value node's Source dropdown) instead of its GUID.
    const QVariantHash identity{{"category", "identity"}};
    addProperty("id", "Name", GizmoProperty::Text, QString(), identity);

    // Free-form layout geometry, common to every gizmo. Tagged "layout" so the
    // inspector can group/separate it from gizmo-specific config later.
    const QVariantHash layout{{"category", "layout"}};
    addProperty("x",        "X",        GizmoProperty::Number, 20.0,  layout);
    addProperty("y",        "Y",        GizmoProperty::Number, 20.0,  layout);
    addProperty("width",    "Width",    GizmoProperty::Number, 160.0, layout);
    addProperty("height",   "Height",   GizmoProperty::Number, 60.0,  layout);
    addProperty("rotation", "Rotation", GizmoProperty::Number, 0.0,   layout);
}

SurfaceGizmo::~SurfaceGizmo()
{
    qDeleteAll(m_impl->properties);
    delete m_impl;
}

GizmoProperty *SurfaceGizmo::addProperty(const QByteArray &t_id, const QString &t_name, GizmoProperty::Type t_type,
                                         const QVariant &t_default, const QVariantHash &t_metadata)
{
    auto *prop = new GizmoProperty(t_id, t_name, t_type, t_default, t_metadata);
    m_impl->properties.append(prop);
    m_impl->propertyById.insert(t_id, prop);
    return prop;
}

GizmoProperty *SurfaceGizmo::property(const QByteArray &t_id) const
{
    return m_impl->propertyById.value(t_id, nullptr);
}

const QVector<GizmoProperty*> &SurfaceGizmo::properties() const
{
    return m_impl->properties;
}

QVariant SurfaceGizmo::propertyValue(const QByteArray &t_id) const
{
    auto *prop = property(t_id);
    return prop ? prop->value() : QVariant();
}

void SurfaceGizmo::setPropertyValue(const QByteArray &t_id, const QVariant &t_value)
{
    auto *prop = property(t_id);
    if(!prop || prop->value() == t_value)
        return;
    prop->setValue(t_value);
    emit propertyChanged(t_id);
    emit valuesChanged();
    markChanged();
}

QString SurfaceGizmo::gizmoTypeString() const
{
    return QString::fromUtf8(m_impl->type);
}

QVariantMap SurfaceGizmo::valuesMap() const
{
    QVariantMap map;
    for(auto *prop : m_impl->properties)
        map.insert(QString::fromUtf8(prop->id()), prop->value());
    return map;
}

void SurfaceGizmo::setPropValue(const QString &t_id, const QVariant &t_value)
{
    setPropertyValue(t_id.toUtf8(), t_value);
}

static QString gizmoPropertyTypeName(GizmoProperty::Type t_type)
{
    switch(t_type)
    {
    case GizmoProperty::Number:  return "Number";
    case GizmoProperty::Boolean: return "Boolean";
    case GizmoProperty::Text:    return "Text";
    case GizmoProperty::Color:   return "Color";
    case GizmoProperty::Point:   return "Point";
    case GizmoProperty::Options: return "Options";
    }
    return "Number";
}

QVariantList SurfaceGizmo::propertyDefs() const
{
    QVariantList defs;
    for(auto *prop : m_impl->properties)
    {
        QVariantMap def;
        def.insert("id", QString::fromUtf8(prop->id()));
        def.insert("name", prop->name());
        def.insert("type", gizmoPropertyTypeName(prop->type()));
        def.insert("category", prop->metadata("category", "general"));
        def.insert("metadata", prop->metadata());
        defs.append(def);
    }
    return defs;
}



void SurfaceGizmo::addHistoryEvent(const QByteArray &id, const QVariant &value)
{
    SurfaceGizmo::GizmoHistoryEvent event;
    event.id = id;
    event.value = value;
    event.time = QDateTime::currentMSecsSinceEpoch()/1000.0;

    m_impl->histroy.prepend(event);

    if(m_impl->histroy.length() > 10)
        m_impl->histroy.pop_back();
}

SurfaceGizmo::GizmoHistoryEvent SurfaceGizmo::lastHistoryEvent() const
{
    if(m_impl->histroy.isEmpty())
        return SurfaceGizmo::GizmoHistoryEvent();
    return m_impl->histroy.front();
}

QVector<SurfaceGizmo::GizmoHistoryEvent> SurfaceGizmo::historyEvents() const
{
    return m_impl->histroy;
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
    setPropertyValue("id", QString::fromUtf8(t_value));
}

QByteArray SurfaceGizmo::id() const
{
    return propertyValue("id").toString().toUtf8();
}

void SurfaceGizmo::markChanged()
{
    emit gizmoUpdated();
}

QVector<SurfaceGizmo::GizmoOutput> SurfaceGizmo::outputs() const
{
    return {};
}

QVariant SurfaceGizmo::outputValue(const QByteArray &) const
{
    return {};
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

    if(m_impl->uniqueId.isEmpty())
        m_impl->uniqueId = QUuid::createUuid().toString().toLatin1();

    const QJsonArray propArray = t_json.value("properties").toArray();
    for(const auto &propValue : propArray)
    {
        const QJsonObject propObj = propValue.toObject();
        if(auto *prop = property(propObj.value("id").toString().toLatin1()))
            prop->readFromJson(propObj);
    }
}

void SurfaceGizmo::writeToJson(QJsonObject &t_json) const
{
    t_json.insert("type", QString(m_impl->type));
    t_json.insert("uniqueId", QString(m_impl->uniqueId));
    t_json.insert("name", QString(m_impl->name));

    QJsonArray propArray;
    for(auto *prop : m_impl->properties)
    {
        QJsonObject propObj;
        prop->writeToJson(propObj);
        propArray.append(propObj);
    }
    t_json.insert("properties", propArray);
}


} // namespace photon
