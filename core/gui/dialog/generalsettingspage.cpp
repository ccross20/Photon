#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include "generalsettingspage.h"
#include "settings/applicationsettings.h"
#include "photoncore.h"
#include "library/songlibrary.h"

namespace photon {

GeneralSettingsPage::GeneralSettingsPage(QWidget *t_parent)
    : AppSettingsPage("General", t_parent)
{
    m_songDataLibraryPathEdit = new QLineEdit;
    m_songDataLibraryPathEdit->setText(ApplicationSettings::songDataLibraryPath());
    connect(m_songDataLibraryPathEdit, &QLineEdit::editingFinished, this, &GeneralSettingsPage::songDataLibraryPathEdited);

    auto *browseButton = new QPushButton("Browse...");
    connect(browseButton, &QPushButton::clicked, this, &GeneralSettingsPage::browseSongDataLibraryPath);

    auto *pathRow = new QHBoxLayout;
    pathRow->addWidget(m_songDataLibraryPathEdit);
    pathRow->addWidget(browseButton);

    auto *form = new QFormLayout;
    form->addRow("Song Data Library", pathRow);

    setLayout(form);
}

void GeneralSettingsPage::songDataLibraryPathEdited()
{
    const QString path = m_songDataLibraryPathEdit->text();
    ApplicationSettings::setSongDataLibraryPath(path);

    if(path.isEmpty())
        photonApp->songLibrary()->close();
    else
        photonApp->songLibrary()->open(path);
}

void GeneralSettingsPage::browseSongDataLibraryPath()
{
    const QString path = QFileDialog::getExistingDirectory(this, "Song Data Library",
                                                             m_songDataLibraryPathEdit->text());
    if(path.isEmpty())
        return;

    m_songDataLibraryPathEdit->setText(path);
    ApplicationSettings::setSongDataLibraryPath(path);
    photonApp->songLibrary()->open(path);
}

} // namespace photon
