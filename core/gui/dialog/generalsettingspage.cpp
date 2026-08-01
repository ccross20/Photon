#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include "generalsettingspage.h"
#include "settings/applicationsettings.h"

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
    ApplicationSettings::setSongDataLibraryPath(m_songDataLibraryPathEdit->text());
}

void GeneralSettingsPage::browseSongDataLibraryPath()
{
    const QString path = QFileDialog::getExistingDirectory(this, "Song Data Library",
                                                             m_songDataLibraryPathEdit->text());
    if(path.isEmpty())
        return;

    m_songDataLibraryPathEdit->setText(path);
    ApplicationSettings::setSongDataLibraryPath(path);
}

} // namespace photon
