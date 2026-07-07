#include <QJsonArray>
#include "containergizmo.h"
#include "gizmofactory.h"

namespace photon {

const QByteArray ContainerGizmo::GizmoId = "Container";

ContainerGizmo::ContainerGizmo() : SurfaceGizmo(GizmoId)
{
    addProperty("layout", "Layout", GizmoProperty::Options, QString("Absolute"),
                {{"options", QStringList{"Absolute", "Vertical", "Horizontal", "Grid"}}});
    addProperty("spacing", "Spacing", GizmoProperty::Number, 6.0);
    addProperty("columns", "Columns", GizmoProperty::Number, 2.0);
    addProperty("padding", "Padding", GizmoProperty::Number, 8.0);
    // When set, layout children share the container's extent equally instead of
    // using their own width/height (ignored by the Absolute layout).
    addProperty("stretch", "Stretch", GizmoProperty::Boolean, false);

    // Containers default larger than a single control.
    setPropertyValue("width", 320.0);
    setPropertyValue("height", 220.0);
}

ContainerGizmo::~ContainerGizmo()
{
    qDeleteAll(m_children);
}

const QVector<SurfaceGizmo*> &ContainerGizmo::children() const
{
    return m_children;
}

void ContainerGizmo::addChild(SurfaceGizmo *t_child, int t_index)
{
    if(!t_child || m_children.contains(t_child))
        return;

    t_child->setParent(this);
    if(t_index < 0 || t_index > m_children.size())
        m_children.append(t_child);
    else
        m_children.insert(t_index, t_child);

    emit childrenChanged();
    markChanged();
}

void ContainerGizmo::removeChild(SurfaceGizmo *t_child)
{
    if(m_children.removeOne(t_child))
    {
        t_child->setParent(nullptr);
        emit childrenChanged();
        markChanged();
    }
}

void ContainerGizmo::collectDescendants(QVector<SurfaceGizmo*> &t_out, bool t_includeContainers) const
{
    for(auto *child : m_children)
    {
        if(auto *container = qobject_cast<ContainerGizmo*>(child))
        {
            if(t_includeContainers)
                t_out.append(container);
            container->collectDescendants(t_out, t_includeContainers);
        }
        else
        {
            t_out.append(child);
        }
    }
}

SurfaceGizmo *ContainerGizmo::findDescendantWithUniqueId(const QByteArray &t_id) const
{
    for(auto *child : m_children)
    {
        if(child->uniqueId() == t_id)
            return child;
        if(auto *container = qobject_cast<ContainerGizmo*>(child))
        {
            if(auto *found = container->findDescendantWithUniqueId(t_id))
                return found;
        }
    }
    return nullptr;
}

ContainerGizmo *ContainerGizmo::parentContainerOf(SurfaceGizmo *t_gizmo) const
{
    if(m_children.contains(t_gizmo))
        return const_cast<ContainerGizmo*>(this);

    for(auto *child : m_children)
    {
        if(auto *container = qobject_cast<ContainerGizmo*>(child))
        {
            if(auto *found = container->parentContainerOf(t_gizmo))
                return found;
        }
    }
    return nullptr;
}

QVariantList ContainerGizmo::childItemsList() const
{
    QVariantList list;
    for(auto *child : m_children)
        list.append(QVariant::fromValue(static_cast<QObject*>(child)));
    return list;
}

void ContainerGizmo::restore(Project &t_project)
{
    SurfaceGizmo::restore(t_project);
    for(auto *child : m_children)
        child->restore(t_project);
}

void ContainerGizmo::readFromJson(const QJsonObject &t_json, const LoadContext &t_context)
{
    SurfaceGizmo::readFromJson(t_json, t_context);

    const QJsonArray childArray = t_json.value("children").toArray();
    for(const auto &childValue : childArray)
    {
        const QJsonObject childObj = childValue.toObject();
        SurfaceGizmo *child = GizmoFactory::createGizmo(childObj.value("type").toString().toLatin1());
        if(!child)
            continue;
        child->readFromJson(childObj, t_context);
        addChild(child);
    }
}

void ContainerGizmo::writeToJson(QJsonObject &t_json) const
{
    SurfaceGizmo::writeToJson(t_json);

    QJsonArray childArray;
    for(auto *child : m_children)
    {
        QJsonObject childObj;
        child->writeToJson(childObj);
        childArray.append(childObj);
    }
    t_json.insert("children", childArray);
}

} // namespace photon
