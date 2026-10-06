#ifndef PHOTON_SONGLIBRARYTEST_H
#define PHOTON_SONGLIBRARYTEST_H

#include <QObject>

namespace photon {

// Moving a Song Library between computers: SongLibrary::importFromFolder()
// rebuilding the catalog from the folder, and stored sequence paths
// surviving the move.
class SongLibraryTest : public QObject
{
    Q_OBJECT
public:
    explicit SongLibraryTest(QObject *parent = nullptr);

private slots:
    void initTestCase();
    void importsSongsAndSequencesFromFolder();
    void relinksCopiedDatabase();
};

} // namespace photon

#endif // PHOTON_SONGLIBRARYTEST_H
