#ifndef PHOTON_SCENEMODEL_H
#define PHOTON_SCENEMODEL_H

#include <QAbstractItemModel>
#include "photon-global.h"

namespace photon {

class SceneObject;

class PHOTONCORE_EXPORT SceneModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SceneModel(SceneObject *root, QObject *parent = nullptr);
    virtual ~SceneModel() override;

    SceneObject *rootObject() const;
    QModelIndex indexForObject(SceneObject *object);
    SceneObject *objectForIndex(const QModelIndex &index) const;

    virtual QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    bool objectIsExpanded(SceneObject *);
    void setObjectIsExpanded(SceneObject *, bool value);

    Qt::DropActions supportedDropActions() const override;
    Qt::DropActions supportedDragActions() const override;
    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex & parent) override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

signals:
    void selectionMoved(QModelIndexList indices);

private slots:
    void childWillBeAdded(photon::SceneObject *parent, photon::SceneObject *child);
    void childWasAdded(photon::SceneObject *child);
    void childWillBeRemoved(photon::SceneObject *parent, photon::SceneObject *child);
    void childWasRemoved(photon::SceneObject *child);
    void childWillBeMoved(photon::SceneObject *child, int newIndex);
    void childWasMoved(photon::SceneObject *child);
    void objectWasUpdated(photon::SceneObject *);

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_SCENEMODEL_H
