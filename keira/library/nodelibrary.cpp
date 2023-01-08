#include "nodelibrary.h"

namespace keira {


FolderElement *FolderElement::folderWithName(const QString &name) const
{
    auto result = std::find_if(m_children.cbegin(), m_children.cend(),[name](const NodeTreeElement *t_element){
                     return t_element->name() == name && t_element->isFolder();
                 });
    if(result != m_children.cend())
        return static_cast<FolderElement*>(*result);
    return nullptr;
}

void FolderElement::sortAlphabetically(bool t_recursive)
{
    std::sort(m_children.begin(), m_children.end(),[](NodeTreeElement *a, NodeTreeElement *b){return a->name() < b->name();});
    if(t_recursive)
    {
        for(auto child : m_children)
        {
            if(child->isFolder())
            {
                static_cast<FolderElement*>(child)->sortAlphabetically(t_recursive);
            }
        }
    }

}

class NodeLibrary::Impl
{
public:
    QVector<NodeInformation> nodes;
    QHash<QByteArray, std::function<Parameter*()>> parameters;
};

NodeLibrary::NodeLibrary():m_impl(new Impl)
{

}

NodeLibrary::~NodeLibrary()
{
    delete m_impl;
}

void NodeLibrary::registerParameter(const QByteArray &id, std::function<Parameter*()> callback)
{
    m_impl->parameters.insert(id, callback);
}

Parameter *NodeLibrary::createParameter(const QByteArray &id) const
{
    return m_impl->parameters.value(id)();
}

void NodeLibrary::registerNode(const NodeInformation &info)
{
    m_impl->nodes.append(info);
}

FolderElement *NodeLibrary::createNodeTree(std::function<bool(const NodeInformation &)> t_pred)
{
    FolderElement *rootFolder = new FolderElement("Root");

    for(auto &info : m_impl->nodes)
    {
        if(!t_pred(info))
            continue;

        FolderElement *currentFolder = rootFolder;

        for(const auto &cat : info.categories)
        {
            FolderElement *foundFolder = currentFolder->folderWithName(cat);

            if(!foundFolder)
            {
                foundFolder = new FolderElement(cat);
                currentFolder->addChild(foundFolder);
            }

            currentFolder = foundFolder;
        }

        currentFolder->addChild(new NodeElement(info));
    }
    rootFolder->sortAlphabetically(true);

    return rootFolder;
}

Node *NodeLibrary::createNode(const QByteArray &nodeId)
{
    auto result = std::find_if(m_impl->nodes.cbegin(), m_impl->nodes.cend(),[nodeId](const NodeInformation &t_info){
                     return t_info.nodeId == nodeId;
                 });
    if(result != m_impl->nodes.cend())
    {
        auto node = (*result).callback();
        node->setName((*result).name);
        node->createParameters();
        return node;
    }

    qWarning() << "Could not create node:" << nodeId;
    return nullptr;

}

} // namespace keira
