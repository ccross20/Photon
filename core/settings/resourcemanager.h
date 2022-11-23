#ifndef RESOURCES_H
#define RESOURCES_H

#include <QScopedPointer>
#include <QObject>
#include <QJsonDocument>
#include <QVector>
#include "photon-global.h"

namespace photon {


class PHOTONCORE_EXPORT Resource
{
public:
    enum ResourceType
    {
        ResourceStyle,
        ResourceSetting,
        ResourceMenu,
        ResourceIcons,
        ResourceLocalization,
        ResourceLayout
    };

    Resource(const QString &path, ResourceType type);

    QString loadToString() const;
    QJsonDocument loadToJson() const;

    QString folderPath() const{return m_folderPath;}
    ResourceType type() const {return m_type;}
    QString path() const{return m_path;}

private:
    QString m_path;
    ResourceType m_type;
    QString m_folderPath;

};

using ResourceList = QVector<Resource>;

class PHOTONCORE_EXPORT ResourceManager : public QObject
{
    Q_OBJECT
public:

    ResourceManager();
    ~ResourceManager();

    void addResource(const Resource &resource);
    void addResource(const QString &path, Resource::ResourceType type);
    void mergeResources(const ResourceManager &resources);

    ResourceList resourcesForType(Resource::ResourceType type) const;

    static QString loadToString(const Resource &resource, bool *error);
    static QJsonDocument loadToJson(const Resource &resource, bool *error);

signals:
    void resourceAdded(const photon::Resource &resource);

private:

    class Impl;
    QScopedPointer<Impl> const m_impl;
};

} // namespace deco

#endif // RESOURCES_H
