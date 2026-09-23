#ifndef PHOTON_STARTUPDIALOG_H
#define PHOTON_STARTUPDIALOG_H

#include <QDialog>
#include "photon-global.h"

class QListWidgetItem;

namespace photon {

// Shown once at launch: quick entry points to start a new project, load one
// from disk, or jump straight into a recently opened/saved one. There's no
// OK/Cancel transaction here - every action closes the dialog once it's
// actually carried out (a cancelled file-open leaves the dialog open so the
// user can try again or pick something else).
class PHOTONCORE_EXPORT StartupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StartupDialog(QWidget *t_parent = nullptr);
    ~StartupDialog();

private slots:
    void newProjectClicked();
    void loadProjectClicked();
    void recentProjectClicked(QListWidgetItem *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_STARTUPDIALOG_H
