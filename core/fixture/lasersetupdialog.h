#ifndef PHOTON_LASERSETUPDIALOG_H
#define PHOTON_LASERSETUPDIALOG_H

#include <QDialog>
#include "photon-global.h"

namespace photon {

// Holds a laser in its setup profile for as long as the dialog is open (see
// OutputOverridesNode) and edits the fixture's setup-only values live: master
// intensity, test frame and projection geometry. Save keeps the edits,
// Cancel restores what the fixture had when the dialog opened.
class PHOTONCORE_EXPORT LaserSetupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LaserSetupDialog(Fixture *fixture, QWidget *parent = nullptr);
    ~LaserSetupDialog();

    void done(int result) override;

private:
    void pushValues();
    void resetValues();
    void updateStatus();

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_LASERSETUPDIALOG_H
