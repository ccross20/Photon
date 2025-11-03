#ifndef COMMONMODEL_H
#define COMMONMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include "photon-global.h"

namespace photon {


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
    FolderData *m_subChannelFolder;
};


class FalloffEffectData : public AbstractTreeData
{
public:
    FalloffEffectData(FalloffEffect*);
    FalloffEffect *effect() const{return m_effect;}

private:
    FalloffEffect *m_effect;
};


class MaskEffectData : public AbstractTreeData
{
public:
    MaskEffectData(MaskEffect*);
    MaskEffect *effect() const{return m_effect;}

private:
    MaskEffect *m_effect;

};

class BaseEffectData : public AbstractTreeData
{
    Q_OBJECT
public:
    BaseEffectData(BaseEffect*);
    BaseEffect *effect() const{return m_effect;}

private slots:
    void channelAdded(photon::Channel *);
    void channelRemoved(photon::Channel *);
    void channelMoved(photon::Channel *);

private:
    BaseEffect *m_effect;

};


} // namespace photon

#endif // COMMONMODEL_H
