#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QInputDialog>
#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include "fixturegroupcollectionpanel_p.h"
#include "photoncore.h"
#include "project/project.h"
#include "fixture/fixturegroup.h"
#include "scene/scenezone.h"

namespace photon {

FixtureGroupCollectionModel::FixtureGroupCollectionModel(FixtureGroupCollection *t_collection)
    : QAbstractItemModel(), m_collection(t_collection)
{
    connect(m_collection, &FixtureGroupCollection::groupWillBeAdded, this, &FixtureGroupCollectionModel::groupWillBeAdded);
    connect(m_collection, &FixtureGroupCollection::groupWasAdded, this, &FixtureGroupCollectionModel::groupWasAdded);
    connect(m_collection, &FixtureGroupCollection::groupWillBeRemoved, this, &FixtureGroupCollectionModel::groupWillBeRemoved);
    connect(m_collection, &FixtureGroupCollection::groupWasRemoved, this, &FixtureGroupCollectionModel::groupWasRemoved);
    connect(m_collection, &FixtureGroupCollection::groupChanged, this, &FixtureGroupCollectionModel::groupChanged);
}

void FixtureGroupCollectionModel::groupWillBeAdded(int t_index)
{
    beginInsertRows(QModelIndex(), t_index, t_index);
}

void FixtureGroupCollectionModel::groupWasAdded(int)
{
    endInsertRows();
}

void FixtureGroupCollectionModel::groupWillBeRemoved(int t_index)
{
    beginRemoveRows(QModelIndex(), t_index, t_index);
}

void FixtureGroupCollectionModel::groupWasRemoved(int)
{
    endRemoveRows();
}

void FixtureGroupCollectionModel::groupChanged(int t_index)
{
    const QModelIndex idx = index(t_index, 0);
    emit dataChanged(idx, idx);
}

int FixtureGroupCollectionModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QModelIndex FixtureGroupCollectionModel::index(int row, int column, const QModelIndex &) const
{
    return createIndex(row, column, m_collection->groupAtIndex(row));
}

QModelIndex FixtureGroupCollectionModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

Qt::ItemFlags FixtureGroupCollectionModel::flags(const QModelIndex &) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QVariant FixtureGroupCollectionModel::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole)
        return QVariant();
    FixtureGroup *group = m_collection->groupAtIndex(index.row());
    if(!group)
        return QVariant();

    QString label = group->name;
    if(!group->query.isEmpty())
    {
        QStringList parts;
        if(!group->query.type.trimmed().isEmpty())
            parts << group->query.type.trimmed();
        if(!group->query.tags.isEmpty())
            parts << group->query.tags.join(' ');
        if(!group->query.zone.trimmed().isEmpty())
            parts << "@" + group->query.zone.trimmed();
        label += "  (" + parts.join(", ") + ")";
    }
    return label;
}

QVariant FixtureGroupCollectionModel::headerData(int, Qt::Orientation, int role) const
{
    if(role == Qt::DisplayRole)
        return "Name";
    return QVariant();
}

int FixtureGroupCollectionModel::rowCount(const QModelIndex &) const
{
    return m_collection->groupCount();
}

FixtureGroupCollectionPanel::FixtureGroupCollectionPanel() : Panel("photon.fixture-group-collection"), m_impl(new Impl)
{
    QVBoxLayout *vLayout = new QVBoxLayout;

    setName("Fixture Groups");
    m_impl->listView = new QListView;
    m_impl->listView->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));

    vLayout->addWidget(m_impl->listView);

    m_impl->addButton = new QPushButton("Add");
    m_impl->addButton->setEnabled(false);
    m_impl->removeButton = new QPushButton("Remove");
    m_impl->removeButton->setEnabled(false);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(m_impl->addButton);
    hLayout->addWidget(m_impl->removeButton);

    vLayout->addLayout(hLayout);

    setPanelLayout(vLayout);

    connect(m_impl->addButton, &QPushButton::clicked, this, &FixtureGroupCollectionPanel::addClicked);
    connect(m_impl->removeButton, &QPushButton::clicked, this, &FixtureGroupCollectionPanel::removeClicked);
    connect(m_impl->listView, &QListView::doubleClicked, this, &FixtureGroupCollectionPanel::doubleClicked);
}

FixtureGroupCollectionPanel::~FixtureGroupCollectionPanel()
{
    delete m_impl;
}

void FixtureGroupCollectionPanel::addClicked()
{
    bool ok = false;
    const QString name = QInputDialog::getText(this, "New Group", "Name:", QLineEdit::Normal, "", &ok);
    if(ok && !name.trimmed().isEmpty())
        photonApp->project()->groups()->addGroup(name.trimmed());
}

void FixtureGroupCollectionPanel::removeClicked()
{
    const auto indicies = m_impl->listView->selectionModel()->selectedRows();
    QStringList toDelete;
    for(const auto &index : indicies)
    {
        if(FixtureGroup *g = static_cast<FixtureGroup*>(index.internalPointer()))
            toDelete.append(g->name);
    }
    for(const QString &name : toDelete)
        photonApp->project()->groups()->removeGroup(name);
}

void FixtureGroupCollectionPanel::doubleClicked(const QModelIndex &t_index)
{
    FixtureGroup *group = static_cast<FixtureGroup*>(t_index.internalPointer());
    if(!group)
        return;

    QDialog dialog(this);
    dialog.setWindowTitle("Edit Group: " + group->name);

    QFormLayout *form = new QFormLayout;
    QLineEdit *typeEdit = new QLineEdit(group->query.type);
    QLineEdit *tagsEdit = new QLineEdit(group->query.tags.join(' '));
    typeEdit->setPlaceholderText("e.g. mover, par (matches type / category)");
    tagsEdit->setPlaceholderText("space-separated tags (fixture must have all)");

    QComboBox *zoneCombo = new QComboBox;
    zoneCombo->addItem("(none)", "");
    for(const QString &z : SceneZone::zoneNames(photonApp->project()))
        zoneCombo->addItem(z, z);
    zoneCombo->setCurrentIndex(qMax(0, zoneCombo->findData(group->query.zone)));

    form->addRow("Type", typeEdit);
    form->addRow("Tags", tagsEdit);
    form->addRow("Zone", zoneCombo);

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->addLayout(form);
    layout->addWidget(buttons);

    if(dialog.exec() == QDialog::Accepted)
    {
        group->query.type = typeEdit->text().trimmed();
        group->query.tags = tagsEdit->text().split(' ', Qt::SkipEmptyParts);
        group->query.zone = zoneCombo->currentData().toString();
        photonApp->project()->groups()->notifyChanged(group);
    }
}

void FixtureGroupCollectionPanel::selectionChanged(const QItemSelection &, const QItemSelection &)
{
    m_impl->removeButton->setEnabled(m_impl->listView->selectionModel()->hasSelection());
}

void FixtureGroupCollectionPanel::projectDidOpen(photon::Project* project)
{
    FixtureGroupCollectionModel *model = new FixtureGroupCollectionModel(project->groups());
    m_impl->listView->setModel(model);

    m_impl->addButton->setEnabled(true);
    connect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FixtureGroupCollectionPanel::selectionChanged);
}

void FixtureGroupCollectionPanel::projectWillClose(photon::Project*)
{
    if(m_impl->listView->selectionModel())
        disconnect(m_impl->listView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FixtureGroupCollectionPanel::selectionChanged);
    m_impl->listView->setModel(nullptr);
    m_impl->addButton->setEnabled(false);
    m_impl->removeButton->setEnabled(false);
}

} // namespace photon
