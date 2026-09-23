#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QPixmap>
#include <QFileInfo>
#include "startupdialog.h"
#include "project/project.h"
#include "photoncore.h"

namespace photon {

class StartupDialog::Impl
{
public:
    QListWidget *recentList;
};

StartupDialog::StartupDialog(QWidget *t_parent) : QDialog(t_parent), m_impl(new Impl)
{
    setWindowTitle("Welcome to Photon");

    auto *logoLabel = new QLabel;
    QPixmap logo(":/icon.png");
    if(!logo.isNull())
        logoLabel->setPixmap(logo.scaledToHeight(96, Qt::SmoothTransformation));
    logoLabel->setAlignment(Qt::AlignCenter);

    auto *newButton = new QPushButton("New Project");
    auto *loadButton = new QPushButton("Load Project...");
    for(auto *button : {newButton, loadButton})
    {
        button->setMinimumHeight(64);
        QFont font = button->font();
        font.setPointSize(font.pointSize() + 2);
        font.setBold(true);
        button->setFont(font);
    }
    connect(newButton, &QPushButton::clicked, this, &StartupDialog::newProjectClicked);
    connect(loadButton, &QPushButton::clicked, this, &StartupDialog::loadProjectClicked);

    auto *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(newButton);
    buttonLayout->addWidget(loadButton);

    auto *recentLabel = new QLabel("Recent Projects");

    m_impl->recentList = new QListWidget;
    connect(m_impl->recentList, &QListWidget::itemClicked, this, &StartupDialog::recentProjectClicked);

    const QStringList recents = Project::recentProjects();
    for(const QString &path : recents)
    {
        auto *item = new QListWidgetItem(QFileInfo(path).fileName());
        item->setToolTip(path);
        item->setData(Qt::UserRole, path);
        m_impl->recentList->addItem(item);
    }

    // Nothing to jump back into yet (first launch, or every recent file has
    // since moved/been deleted) - just the two big entry points above.
    const bool hasRecents = !recents.isEmpty();
    recentLabel->setVisible(hasRecents);
    m_impl->recentList->setVisible(hasRecents);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(logoLabel);
    mainLayout->addSpacing(8);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(recentLabel);
    mainLayout->addWidget(m_impl->recentList, 1);
    setLayout(mainLayout);

    resize(420, hasRecents ? 480 : 220);
}

StartupDialog::~StartupDialog()
{
    delete m_impl;
}

void StartupDialog::newProjectClicked()
{
    photonApp->newProject();
    accept();
}

void StartupDialog::loadProjectClicked()
{
    // Only close on an actual load - a cancelled file-open dialog shouldn't
    // also dismiss this one, or the user has to reopen it from the File menu
    // just to try again.
    if(photonApp->loadProject())
        accept();
}

void StartupDialog::recentProjectClicked(QListWidgetItem *t_item)
{
    const QString path = t_item->data(Qt::UserRole).toString();
    if(photonApp->loadProject(path))
        accept();
}

} // namespace photon
