#ifndef PHOTON_GENERALSETTINGSPAGE_H
#define PHOTON_GENERALSETTINGSPAGE_H

#include "appsettingspage.h"

class QLineEdit;

namespace photon {

class PHOTONCORE_EXPORT GeneralSettingsPage : public AppSettingsPage
{
    Q_OBJECT
public:
    explicit GeneralSettingsPage(QWidget *t_parent = nullptr);

private slots:
    void songDataLibraryPathEdited();
    void browseSongDataLibraryPath();

private:
    QLineEdit *m_songDataLibraryPathEdit = nullptr;
};

} // namespace photon

#endif // PHOTON_GENERALSETTINGSPAGE_H
