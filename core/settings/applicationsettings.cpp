#include <QSettings>
#include "applicationsettings.h"

namespace photon {

namespace {
    const char *kGroup = "Settings";
    const char *kSongDataLibraryPath = "songDataLibraryPath";
}

QString ApplicationSettings::songDataLibraryPath()
{
    QSettings settings;
    settings.beginGroup(kGroup);
    QString value = settings.value(kSongDataLibraryPath).toString();
    settings.endGroup();
    return value;
}

void ApplicationSettings::setSongDataLibraryPath(const QString &t_path)
{
    QSettings settings;
    settings.beginGroup(kGroup);
    settings.setValue(kSongDataLibraryPath, t_path);
    settings.endGroup();
}

} // namespace photon
