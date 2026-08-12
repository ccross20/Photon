#include <QJsonArray>
#include <QJsonObject>
#include "projectresource.h"
#include "gui/resourceeditorwidget.h"

namespace photon {

ProjectResourceNotifier::ProjectResourceNotifier(ProjectResource *t_resource, QObject *parent)
    : QObject{parent}, m_resource(t_resource)
{
}

ProjectResource *ProjectResourceNotifier::resource() const
{
    return m_resource;
}

ProjectResource::ProjectResource() : m_notifier(new ProjectResourceNotifier(this))
{
}

ProjectResource::~ProjectResource()
{
    delete m_notifier;
}

ProjectResourceNotifier *ProjectResource::resourceNotifier() const
{
    return m_notifier;
}

void ProjectResource::notifyResourceChanged()
{
    emit m_notifier->resourceChanged(this);
}

QWidget *ProjectResource::createResourceEditor()
{
    return new ResourceEditorWidget(this);
}

const QStringList &ProjectResource::resourceTags() const
{
    return m_tags;
}

void ProjectResource::setResourceTags(const QStringList &t_tags)
{
    QStringList normalised;
    normalised.reserve(t_tags.size());
    for(const auto &tag : t_tags)
    {
        const QString lower = tag.toLower().trimmed();
        if(!lower.isEmpty() && !normalised.contains(lower))
            normalised.append(lower);
    }
    normalised.sort();

    if(normalised == m_tags)
        return;

    m_tags = normalised;
    notifyResourceChanged();
}

void ProjectResource::addResourceTag(const QString &t_tag)
{
    const QString lower = t_tag.toLower().trimmed();
    if(lower.isEmpty() || m_tags.contains(lower))
        return;

    m_tags.append(lower);
    m_tags.sort();
    notifyResourceChanged();
}

void ProjectResource::removeResourceTag(const QString &t_tag)
{
    const QString lower = t_tag.toLower().trimmed();
    if(!m_tags.removeOne(lower))
        return;

    notifyResourceChanged();
}

bool ProjectResource::hasResourceTag(const QString &t_tag) const
{
    return m_tags.contains(t_tag.toLower().trimmed());
}

void ProjectResource::readResourceJson(const QJsonObject &t_json)
{
    if(!t_json.contains("tags"))
        return;

    QStringList tags;
    for(const auto &tag : t_json.value("tags").toArray())
        tags << tag.toString();
    setResourceTags(tags);
}

void ProjectResource::writeResourceJson(QJsonObject &t_json) const
{
    if(m_tags.isEmpty())
        return;

    QJsonArray tagArray;
    for(const auto &tag : m_tags)
        tagArray.append(tag);
    t_json.insert("tags", tagArray);
}

} // namespace photon
