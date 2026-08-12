#ifndef PHOTON_PROJECTMODEL_H
#define PHOTON_PROJECTMODEL_H

#include <QAbstractItemModel>
#include "photon-global.h"
#include "project/projectresource.h"

namespace photon {

// A top-level grouping row in the project tree ("Rig", "Routines", ...). It is
// itself a ProjectResource so every index's internalPointer is one type and
// folders are told apart with a dynamic_cast rather than a side table.
class PHOTONCORE_EXPORT ProjectFolder : public QObject, public ProjectResource
{
    Q_OBJECT
public:
    ProjectFolder(const QString &name, const QByteArray &contentTypeId, QObject *parent = nullptr);

    // The resourceTypeId of what this folder holds - "routine", "surface", ...
    // The Rig folder holds scene objects of many types and reports "scene".
    QByteArray contentTypeId() const;

    QByteArray resourceId() const override;
    QByteArray resourceTypeId() const override{return "folder";}
    QString resourceName() const override{return m_name;}
    void setResourceName(const QString &) override{}   // folders aren't renameable
    QObject *resourceObject() override{return this;}
    QWidget *createResourceEditor() override{return nullptr;}

private:
    QString m_name;
    QByteArray m_contentTypeId;
};

// One tree over every asset in the project. Top-level rows are folders; the Rig
// folder's subtree is the live SceneObject hierarchy, walked directly rather
// than mirrored, exactly as SceneModel does.
class PHOTONCORE_EXPORT ProjectModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Column
    {
        NameColumn = 0,
        VisibleColumn,
        TagsColumn,
        ColumnCount
    };

    enum Role
    {
        // Space-joined tags, for the filter proxy to match against.
        TagsRole = Qt::UserRole + 1,
        // resourceTypeId of the row.
        TypeRole,
        // True for the top-level grouping rows.
        IsFolderRole
    };

    explicit ProjectModel(Project *project, QObject *parent = nullptr);
    ~ProjectModel() override;

    ProjectResource *resourceForIndex(const QModelIndex &) const;
    QModelIndex indexForResource(ProjectResource *) const;
    // The folder a resource belongs to, or nullptr for the root/unknown.
    ProjectFolder *folderForContentType(const QByteArray &contentTypeId) const;

    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

signals:
    // Emitted after a drag reparents objects, so the view can restore the
    // selection onto their new indices.
    void selectionMoved(QModelIndexList indices);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_PROJECTMODEL_H
