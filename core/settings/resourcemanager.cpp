#include <QHash>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include "resourcemanager.h"

namespace photon {

Resource::Resource(const QString &path, ResourceType type) : m_path(path),m_type(type)
{
    QDir dir = QFileInfo(path).absoluteDir();
    m_folderPath = dir.path();
}

QString Resource::loadToString() const
{
    QFile file(m_path);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);

        return in.readAll();

    } else {

        qWarning() << "Error opening file: " << m_path;
        return "";
    }
}

QJsonDocument Resource::loadToJson() const
{
    const QString &data = loadToString();
    return QJsonDocument::fromJson(data.toLatin1());
}

class ResourceManager::Impl{
public:
    Impl();
    ~Impl();
    QMultiHash<Resource::ResourceType, Resource> resourceHash;
};

ResourceManager::Impl::Impl()
{

}

ResourceManager::Impl::~Impl()
{

}

ResourceManager::ResourceManager() : m_impl(new ResourceManager::Impl())
{

}

ResourceManager::~ResourceManager()
{

}

void ResourceManager::addResource(const QString &path, Resource::ResourceType type)
{
    //qDebug() << "Add resource" << path;
    addResource(Resource(path, type));
}

void ResourceManager::addResource(const Resource &resource)
{
    m_impl->resourceHash.insert(resource.type(), resource);

    emit resourceAdded(resource);
}

void ResourceManager::mergeResources(const ResourceManager &resources)
{
    QMultiHash<Resource::ResourceType, Resource>::const_iterator i = resources.m_impl->resourceHash.constBegin();
    while (i != resources.m_impl->resourceHash.constEnd()) {
        addResource(i.value());
        ++i;
    }
}

ResourceList ResourceManager::resourcesForType(Resource::ResourceType type) const
{
    return m_impl->resourceHash.values(type);
}

QString ResourceManager::loadToString(const Resource &resource, bool *error)
{
    QFile file(resource.path());

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        *error = false;

        return in.readAll();

    } else {

        *error = true;
        qWarning() << "Error opening file: " << resource.path();
        return "";
    }
}

QJsonDocument ResourceManager::loadToJson(const Resource &resource, bool *error)
{
    const QString &data = ResourceManager::loadToString(resource, error);
    return QJsonDocument::fromJson(data.toLatin1());
}

} // namespace deco
