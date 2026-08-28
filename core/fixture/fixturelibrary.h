#ifndef PHOTON_FIXTURELIBRARY_H
#define PHOTON_FIXTURELIBRARY_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include "photon-global.h"

namespace photon {

// One fixture-definition file, summarized for browsing/searching - not a
// loaded Fixture (that's a much heavier parse of channels/wheels/modes, done
// on demand only once a definition is actually chosen). manufacturer is a
// best-effort guess from the "Manufacturer-Model.json" filename convention
// this codebase's shipped fixtures follow, since the OpenFixture JSON body
// itself has no manufacturer field.
struct PHOTONCORE_EXPORT FixtureDefinitionInfo
{
    QString path;
    QString name;
    QStringList categories;
    QString manufacturer;
};

// Catalog of the fixture-definition JSON files available to add to a
// project (today: <app dir>/fixtures - see defaultDirectory()). Scanned once
// at startup (see PhotonCore::init()) so the "Add Fixture" picker can show a
// searchable list instead of a raw file dialog.
//
// Parsing every definition's name/categories on every launch is trivial at
// today's library size (a dozen files), but this caches those two fields to
// <appDataPath>/fixture_library_cache.json anyway, keyed by each file's
// size+modified time, so a large user-grown library only re-parses files
// that actually changed. A real database (the way SongLibrary uses SQLite)
// would be overkill here - this catalog is read-only (scanned from disk,
// never authored/mutated in place) and never queried relationally, so a flat
// cache keyed by path is all it needs.
class PHOTONCORE_EXPORT FixtureLibrary : public QObject
{
    Q_OBJECT
public:
    explicit FixtureLibrary(QObject *parent = nullptr);
    ~FixtureLibrary();

    // Where fixture definitions ship from: <applicationDirPath>/fixtures.
    static QString defaultDirectory();

    // Rescans defaultDirectory() for *.json fixture definitions. Safe to call
    // again later (e.g. a manual "Refresh" action) to pick up files dropped
    // in after startup.
    void scan();

    const QVector<FixtureDefinitionInfo> &definitions() const;

signals:
    void scanned();

private:
    QVector<FixtureDefinitionInfo> m_definitions;
};

} // namespace photon

#endif // PHOTON_FIXTURELIBRARY_H
