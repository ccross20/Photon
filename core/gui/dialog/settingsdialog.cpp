#include <QListWidget>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include "settingsdialog.h"
#include "appsettingspage.h"
#include "generalsettingspage.h"

namespace photon {

class SettingsDialog::Impl
{
public:
    QListWidget *pageList;
    QStackedWidget *pageStack;
};

SettingsDialog::SettingsDialog(QWidget *t_parent) : QDialog(t_parent), m_impl(new Impl)
{
    setWindowTitle("Settings");

    m_impl->pageList = new QListWidget;
    m_impl->pageList->setFixedWidth(160);
    m_impl->pageStack = new QStackedWidget;

    connect(m_impl->pageList, &QListWidget::currentRowChanged, this, &SettingsDialog::pageSelectionChanged);

    auto *splitLayout = new QHBoxLayout;
    splitLayout->addWidget(m_impl->pageList);
    splitLayout->addWidget(m_impl->pageStack, 1);

    auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::close);

    auto *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(splitLayout, 1);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    resize(600, 400);

    // First page - future settings pages get added here the same way.
    addPage(new GeneralSettingsPage);

    m_impl->pageList->setCurrentRow(0);
}

SettingsDialog::~SettingsDialog()
{
    delete m_impl;
}

void SettingsDialog::addPage(AppSettingsPage *t_page)
{
    m_impl->pageStack->addWidget(t_page);
    m_impl->pageList->addItem(t_page->pageName());
}

void SettingsDialog::pageSelectionChanged(int t_row)
{
    if(t_row >= 0)
        m_impl->pageStack->setCurrentIndex(t_row);
}

} // namespace photon
