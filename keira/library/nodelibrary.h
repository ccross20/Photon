#ifndef KEIRA_NODELIBRARY_H
#define KEIRA_NODELIBRARY_H

#include "model/node.h"

namespace keira {

class KEIRA_EXPORT NodeTreeElement
{
public:
    NodeTreeElement(const QString &name):m_name(name){}
    virtual ~NodeTreeElement(){}

    QString name() const{return m_name;}
    virtual bool isFolder() const{return false;}
    virtual uint childCount() const{return 0;}
    virtual NodeTreeElement *elementAt(uint index) const{return nullptr;}

private:
    QString m_name;
};

class KEIRA_EXPORT FolderElement : public NodeTreeElement
{
public:
    FolderElement(const QString &name):NodeTreeElement(name){}
    bool isFolder() const override{return true;}
    uint childCount() const override{return m_children.length();}
    NodeTreeElement *elementAt(uint index) const override{return m_children.at(index);}
    FolderElement *folderWithName(const QString &name) const;
    void addChild(NodeTreeElement *element){m_children.append(element);}
    void sortAlphabetically(bool recursive);

private:
    QVector<NodeTreeElement*> m_children;
};

class KEIRA_EXPORT NodeElement : public NodeTreeElement
{
public:
    NodeElement(const NodeInformation &info):NodeTreeElement(info.name),m_info(info){}
    const NodeInformation &info() const{return m_info;}

private:
    NodeInformation m_info;
};

class KEIRA_EXPORT NodeLibrary
{
public:
    NodeLibrary();
    ~NodeLibrary();

    void registerParameter(const QByteArray &id, std::function<Parameter*()> callback);
    Parameter *createParameter(const QByteArray &id) const;
    void registerNode(const NodeInformation &info);
    FolderElement *createNodeTree(std::function<bool(const NodeInformation &)> pred);

    Node *createNode(const QByteArray &nodeId);

    // The nodeId of a registered GraphInputNode that exposes a port of the given
    // parameter typeId, or empty if none. Used to auto-create an input node when a
    // wire is dropped on a subgraph.
    QByteArray inputNodeForParameterType(const QByteArray &parameterType) const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // KEIRA_NODELIBRARY_H
