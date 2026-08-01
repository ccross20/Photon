#ifndef PHOTON_APPLICATIONSETTINGS_H
#define PHOTON_APPLICATIONSETTINGS_H

#include <QString>
#include "photon-global.h"

namespace photon {

// Typed accessors for the app's persisted (QSettings-backed) preferences, edited
// via SettingsDialog. Centralised here so key names live in one place rather
// than being duplicated at every read site.
class PHOTONCORE_EXPORT ApplicationSettings
{
public:
    static QString songDataLibraryPath();
    static void setSongDataLibraryPath(const QString &);
};

} // namespace photon

#endif // PHOTON_APPLICATIONSETTINGS_H
