#ifndef PHOTON_FIXTURELIBRARYDIALOG_H
#define PHOTON_FIXTURELIBRARYDIALOG_H

#include <QDialog>
#include "photon-global.h"

namespace photon {

// Searchable replacement for the old "pick a fixture .json" QFileDialog -
// lists every definition PhotonCore::fixtureLibrary() found (name,
// manufacturer, categories), filterable by a search box.
class PHOTONCORE_EXPORT FixtureLibraryDialog : public QDialog
{
    Q_OBJECT
public:
    explicit FixtureLibraryDialog(QWidget *t_parent = nullptr);
    ~FixtureLibraryDialog();

    // Path of the selected fixture definition, valid once accepted.
    QString selectedPath() const;

    // Runs the dialog modally; returns the chosen definition's path, or an
    // empty string if the user cancelled.
    static QString getFixturePath(QWidget *t_parent = nullptr);

private:
    void updateOkEnabled();

    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_FIXTURELIBRARYDIALOG_H
