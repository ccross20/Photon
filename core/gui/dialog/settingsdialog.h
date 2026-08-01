#ifndef PHOTON_SETTINGSDIALOG_H
#define PHOTON_SETTINGSDIALOG_H

#include <QDialog>
#include "photon-global.h"

namespace photon {

class AppSettingsPage;

// Application-wide preferences: named pages listed on the left, the selected
// page's controls on the right. Each page persists its own values immediately
// via QSettings as the user edits them, so this dialog is just a Close button -
// no Apply/Cancel/OK transaction to manage.
class PHOTONCORE_EXPORT SettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *t_parent = nullptr);
    ~SettingsDialog();

private slots:
    void pageSelectionChanged(int t_row);

private:
    void addPage(AppSettingsPage *);

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_SETTINGSDIALOG_H
