#ifndef PHOTON_CLIPMODEL_H
#define PHOTON_CLIPMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "photon-global.h"

namespace photon {

class FalloffData;
class MaskData;
class StateClip;

class AbstractTreeData : public QObject
{
    Q_OBJECT
public:

    enum TreeDataType
    {
        DataAny,
        DataChannel,
        DataFalloff,
        DataSelection,
        DataState
    };

    AbstractTreeData(const QString &name, const QByteArray &id = QByteArray{});
    virtual ~AbstractTreeData();
    QString name();
    void setName(const QString &name);
    QByteArray id() const{return m_id;}
    void setId(const QByteArray &t_id){m_id = t_id;}

    void addChild(AbstractTreeData*);
    void insertChild(AbstractTreeData*, int index);
    void removeChild(AbstractTreeData*);
    const QVector<AbstractTreeData*> &children() const{return m_children;}
    AbstractTreeData *childAtIndex(int i){return m_children.at(i);}
    int childCount() const{return m_children.length();}
    int index() const{return m_index;}
    AbstractTreeData *findDataWithId(const QByteArray &) const;

signals:
    void childWillBeAdded(photon::AbstractTreeData*, int);
    void childWasAdded(photon::AbstractTreeData*);
    void childWillBeRemoved(photon::AbstractTreeData*, int);
    void childWasRemoved(photon::AbstractTreeData*);
    void metadataUpdated(photon::AbstractTreeData*);

private:
    QVector<AbstractTreeData*> m_children;
    QString m_name;
    QByteArray m_id;
    int m_index;
};

class RootData : public AbstractTreeData
{
public:
    RootData();
};

class FolderData : public AbstractTreeData
{
public:
    FolderData(const QString &name, TreeDataType allowedTypes, bool showCreateItem = true);

    TreeDataType allowedTypes() const{return m_allowedTypes;}

private:
    bool m_showCreate;
    TreeDataType m_allowedTypes;
};

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
    MaskData *m_maskFolder;
    FalloffData *m_falloffData;
    FolderData *m_channelFolder;
    Clip *m_clip;

};

class MasterLayerData : public AbstractTreeData
{
    Q_OBJECT
public:
    MasterLayerData(MasterLayer *);
    MasterLayer *layer() const{return m_layer;}

private:
    FolderData *m_channelFolder;
    MasterLayer *m_layer;

};

class CreateData : public AbstractTreeData
{
public:
    CreateData(const QString &name);

};


class ChannelEffectData : public AbstractTreeData
{
public:
    ChannelEffectData(ChannelEffect*);
    ChannelEffect *effect() const{return m_effect;}

private:
    ChannelEffect *m_effect;
};

class ChannelData : public AbstractTreeData
{
    Q_OBJECT
public:
    ChannelData(Channel*);
    ChannelEffectData *findEffectData(ChannelEffect *);

    Channel *channel() const{return m_channel;}

private slots:
    void effectAdded(photon::ChannelEffect*);
    void effectRemoved(photon::ChannelEffect*);
    void effectMoved(photon::ChannelEffect*);
    void channelUpdated();

private:
    Channel *m_channel;
};


class FalloffEffectData : public AbstractTreeData
{
public:
    FalloffEffectData(FalloffEffect*);
    FalloffEffect *effect() const{return m_effect;}

private:
    FalloffEffect *m_effect;
};

class FalloffData : public AbstractTreeData
{
    Q_OBJECT
public:
    FalloffData(Clip*);
    FalloffEffectData *findEffectData(FalloffEffect *);

    Clip *clip() const{return m_clip;}

private slots:
    void effectAdded(photon::FalloffEffect*);
    void effectRemoved(photon::FalloffEffect*);
    void effectMoved(photon::FalloffEffect*);

private:
    Clip *m_clip;
};

class MaskEffectData : public AbstractTreeData
{
public:
    MaskEffectData(MaskEffect*);
    MaskEffect *effect() const{return m_effect;}

private:
    MaskEffect *m_effect;

};

class MaskData : public AbstractTreeData
{
    Q_OBJECT
public:
    MaskData(Clip*);
    MaskEffectData *findEffectData(MaskEffect *);

    Clip *clip() const{return m_clip;}

private slots:
    void effectAdded(photon::MaskEffect*);
    void effectRemoved(photon::MaskEffect*);
    void effectMoved(photon::MaskEffect*);

private:
    Clip *m_clip;
};




class StateData : public AbstractTreeData
{
public:
    StateData(StateClip*);

    State *state() const;
    StateClip *clip() const{return m_clip;}

private:
    StateClip *m_clip;
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
    void addMasterLayer(MasterLayer *);
    void removeMasterLayer(MasterLayer *);
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
