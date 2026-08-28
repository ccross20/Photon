#include <QDialogButtonBox>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include "fixturelibrarydialog.h"
#include "fixture/fixturelibrary.h"
#include "photoncore.h"

namespace photon {

namespace {
const int PathRole = Qt::UserRole + 1;
const int SearchRole = Qt::UserRole + 2;
}

class FixtureLibraryDialog::Impl
{
public:
    QLineEdit *searchEdit = nullptr;
    QListView *listView = nullptr;
    QStandardItemModel *model = nullptr;
    QSortFilterProxyModel *proxy = nullptr;
    QDialogButtonBox *buttons = nullptr;
    QString selectedPath;
};

FixtureLibraryDialog::FixtureLibraryDialog(QWidget *t_parent) : QDialog(t_parent), m_impl(new Impl)
{
    setWindowTitle("Add Fixture");
    resize(420, 480);

    m_impl->searchEdit = new QLineEdit;
    m_impl->searchEdit->setPlaceholderText("Search fixtures");
    m_impl->searchEdit->setClearButtonEnabled(true);

    m_impl->model = new QStandardItemModel(this);
    for(const FixtureDefinitionInfo &def : photonApp->fixtureLibrary()->definitions())
    {
        QString display = def.name;
        QStringList subtitleParts;
        if(!def.manufacturer.isEmpty())
            subtitleParts << def.manufacturer;
        if(!def.categories.isEmpty())
            subtitleParts << def.categories.join(", ");
        if(!subtitleParts.isEmpty())
            display += "  —  " + subtitleParts.join(" · ");

        auto *item = new QStandardItem(display);
        item->setEditable(false);
        item->setData(def.path, PathRole);
        item->setData(def.name + ' ' + def.manufacturer + ' ' + def.categories.join(' '), SearchRole);
        m_impl->model->appendRow(item);
    }

    m_impl->proxy = new QSortFilterProxyModel(this);
    m_impl->proxy->setSourceModel(m_impl->model);
    m_impl->proxy->setFilterRole(SearchRole);
    m_impl->proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);

    m_impl->listView = new QListView;
    m_impl->listView->setModel(m_impl->proxy);
    m_impl->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_impl->listView->setSelectionMode(QAbstractItemView::SingleSelection);

    m_impl->buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    m_impl->buttons->button(QDialogButtonBox::Ok)->setEnabled(false);

    auto *layout = new QVBoxLayout(this);
    layout->addWidget(m_impl->searchEdit);
    layout->addWidget(m_impl->listView, 1);
    layout->addWidget(m_impl->buttons);

    auto acceptCurrent = [this](){
        const QModelIndex index = m_impl->listView->currentIndex();
        if(!index.isValid())
            return;
        m_impl->selectedPath = index.data(PathRole).toString();
        accept();
    };

    connect(m_impl->searchEdit, &QLineEdit::textChanged, this, [this](const QString &t_text){
        m_impl->proxy->setFilterFixedString(t_text);
        // Typing a search that narrows to one obvious match should leave that
        // match pre-selected, so a follow-up Enter (in the search box) accepts
        // it immediately instead of requiring a separate click into the list.
        if(m_impl->proxy->rowCount() > 0)
            m_impl->listView->setCurrentIndex(m_impl->proxy->index(0, 0));
        updateOkEnabled();
    });
    connect(m_impl->searchEdit, &QLineEdit::returnPressed, this, acceptCurrent);
    connect(m_impl->listView, &QListView::activated, this, acceptCurrent);
    connect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](){
        updateOkEnabled();
    });
    connect(m_impl->buttons, &QDialogButtonBox::accepted, this, acceptCurrent);
    connect(m_impl->buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    m_impl->searchEdit->setFocus();
}

FixtureLibraryDialog::~FixtureLibraryDialog()
{
    delete m_impl;
}

void FixtureLibraryDialog::updateOkEnabled()
{
    m_impl->buttons->button(QDialogButtonBox::Ok)->setEnabled(m_impl->listView->currentIndex().isValid());
}

QString FixtureLibraryDialog::selectedPath() const
{
    return m_impl->selectedPath;
}

QString FixtureLibraryDialog::getFixturePath(QWidget *t_parent)
{
    FixtureLibraryDialog dialog(t_parent);
    if(dialog.exec() == QDialog::Accepted)
        return dialog.selectedPath();
    return QString();
}

} // namespace photon
