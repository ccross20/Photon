#ifndef PHOTON_CLIPMODEL_H
#define PHOTON_CLIPMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "photon-global.h"
#include "data/commonmodel.h"

namespace photon {

class ClipGraphData;


class ClipData : public AbstractTreeData
{
    Q_OBJECT
public:
    ClipData(Clip *);
    Clip *clip() const{return m_clip;}

private slots:
    void channelAdded(photon::Channel *);
    void channelRemoved(photon::Channel *);
    void channelMoved(photon::Channel *);

private:
    FolderData *m_channelFolder;
    ClipGraphData *m_graphData = nullptr;
    Clip *m_clip;

};

// A leaf node for a clip that has a content graph (Clip::contentGraph()) - lets
// the editor show/edit that graph (e.g. FixtureClip's internal FixtureStateNode
// routine) inline.
class ClipGraphData : public AbstractTreeData
{
    Q_OBJECT
public:
    ClipGraphData(Clip*);

    Clip *clip() const{return m_clip;}

private:
    Clip *m_clip;
};

class ClipModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ClipModel();
    ~ClipModel();

    AbstractTreeData *dataForIndex(const QModelIndex &) const;
    QModelIndex indexForData(AbstractTreeData *) const;
    QModelIndex indexForId(const QByteArray &) const;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    void addClip(Clip *);
    void removeClip(Clip *);
    QVector<Clip*> clips() const;
    RootData *root() const{return m_root;}

private slots:
    void childWillBeAdded(photon::AbstractTreeData*, int);
    void childWasAdded(photon::AbstractTreeData*);
    void childWillBeRemoved(photon::AbstractTreeData*, int);
    void childWasRemoved(photon::AbstractTreeData*);
    void metadataUpdated(photon::AbstractTreeData*);

private:
    RootData *m_root;
};

} // namespace photon

#endif // PHOTON_CLIPMODEL_H
