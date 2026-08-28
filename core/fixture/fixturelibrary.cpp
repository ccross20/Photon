#include <algorithm>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include "fixturelibrary.h"
#include "photoncore.h"

namespace photon {

namespace {

QString cachePath()
{
    return photonApp->appDataPath() + "/fixture_library_cache.json";
}

// "Elation-Dartz-360.json" -> "Elation"; falls back to empty (rather than the
// whole filename) when there's no separator to split on, since a wrong guess
// is worse than no manufacturer at all for search purposes.
QString manufacturerFromFileName(const QString &t_fileName)
{
    const QString base = QFileInfo(t_fileName).completeBaseName();
    const int dash = base.indexOf('-');
    if(dash <= 0)
        return QString();
    return base.left(dash);
}

struct CacheEntry
{
    qint64 size = -1;
    qint64 modified = 0;
    QString name;
    QStringList categories;
};

QHash<QString, CacheEntry> readCache()
{
    QHash<QString, CacheEntry> cache;

    QFile cacheFile(cachePath());
    if(!cacheFile.open(QIODevice::ReadOnly))
        return cache;

    const QJsonArray entries = QJsonDocument::fromJson(cacheFile.readAll()).array();
    for(const auto &entryValue : entries)
    {
        const QJsonObject entry = entryValue.toObject();

        CacheEntry cacheEntry;
        cacheEntry.size = entry.value("size").toInteger();
        cacheEntry.modified = entry.value("modified").toInteger();
        cacheEntry.name = entry.value("name").toString();
        for(const auto &c : entry.value("categories").toArray())
            cacheEntry.categories << c.toString();

        cache.insert(entry.value("path").toString(), cacheEntry);
    }

    return cache;
}

} // namespace

FixtureLibrary::FixtureLibrary(QObject *t_parent) : QObject(t_parent)
{
}

FixtureLibrary::~FixtureLibrary()
{
}

QString FixtureLibrary::defaultDirectory()
{
    return QCoreApplication::applicationDirPath() + "/fixtures";
}

void FixtureLibrary::scan()
{
    m_definitions.clear();

    QDir dir(defaultDirectory());
    if(!dir.exists())
    {
        emit scanned();
        return;
    }

    const QHash<QString, CacheEntry> cache = readCache();
    QJsonArray newCache;

    const QStringList fileNames = dir.entryList(QStringList() << "*.json", QDir::Files, QDir::Name);
    for(const QString &fileName : fileNames)
    {
        const QFileInfo info(dir.filePath(fileName));
        const QString path = info.absoluteFilePath();
        const qint64 modified = info.lastModified().toMSecsSinceEpoch();

        QString name;
        QStringList categories;

        const auto cached = cache.constFind(path);
        if(cached != cache.constEnd() && cached->size == info.size() && cached->modified == modified)
        {
            name = cached->name;
            categories = cached->categories;
        }
        else
        {
            QFile file(path);
            if(file.open(QIODevice::ReadOnly))
            {
                const QJsonObject json = QJsonDocument::fromJson(file.readAll()).object();
                name = json.value("name").toString();
                for(const auto &c : json.value("categories").toArray())
                    categories << c.toString();
            }
        }

        if(name.isEmpty())
            name = info.completeBaseName();

        FixtureDefinitionInfo def;
        def.path = path;
        def.name = name;
        def.categories = categories;
        def.manufacturer = manufacturerFromFileName(fileName);
        m_definitions << def;

        QJsonObject cacheEntry;
        cacheEntry.insert("path", path);
        cacheEntry.insert("size", info.size());
        cacheEntry.insert("modified", modified);
        cacheEntry.insert("name", name);
        QJsonArray categoryArray;
        for(const auto &c : categories)
            categoryArray << c;
        cacheEntry.insert("categories", categoryArray);
        newCache << cacheEntry;
    }

    std::sort(m_definitions.begin(), m_definitions.end(), [](const FixtureDefinitionInfo &a, const FixtureDefinitionInfo &b){
        return a.name.compare(b.name, Qt::CaseInsensitive) < 0;
    });

    QFile writeCache(cachePath());
    if(writeCache.open(QIODevice::WriteOnly | QIODevice::Truncate))
        writeCache.write(QJsonDocument(newCache).toJson(QJsonDocument::Compact));

    emit scanned();
}

const QVector<FixtureDefinitionInfo> &FixtureLibrary::definitions() const
{
    return m_definitions;
}

} // namespace photon
