#ifndef PHOTON_MENUFACTORY_H
#define PHOTON_MENUFACTORY_H
#include <QMenu>
#include "photon-global.h"

namespace photon {


class NodeTreeElement
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

class FolderElement : public NodeTreeElement
{
public:
    FolderElement(const QString &name):NodeTreeElement(name){}
    bool isFolder() const override{return true;}
    uint childCount() const override{return m_children.length();}
    NodeTreeElement *elementAt(uint index) const override{return m_children.at(index);}
    FolderElement *folderWithName(const QString &name) const
    {
        auto result = std::find_if(m_children.cbegin(), m_children.cend(),[name](const NodeTreeElement *t_element){
                         return t_element->name() == name && t_element->isFolder();
                     });
        if(result != m_children.cend())
            return static_cast<FolderElement*>(*result);
        return nullptr;
    }
    void addChild(NodeTreeElement *element){m_children.append(element);}
    void sortAlphabetically(bool recursive){
        std::sort(m_children.begin(), m_children.end(),[](NodeTreeElement *a, NodeTreeElement *b){return a->name() < b->name();});
        if(recursive)
        {
            for(auto child : m_children)
            {
                if(child->isFolder())
                {
                    static_cast<FolderElement*>(child)->sortAlphabetically(recursive);
                }
            }
        }
    }

private:
    QVector<NodeTreeElement*> m_children;
};



template<class DataType>
class PHOTONCORE_EXPORT MenuFactory
{
public:

    class NodeElement : public NodeTreeElement
    {
    public:
        NodeElement(const DataType &info, int index):NodeTreeElement(info.name),m_info(info),m_index(index){}
        const DataType &info() const{return m_info;}
        int index() const{return m_index;}

    private:
        DataType m_info;
        int m_index;
    };




    MenuFactory(){rootFolder = new FolderElement("Root");}

    void addItem(QVector<QByteArray> &categories, const DataType &t_data)
    {
        FolderElement *currentFolder = rootFolder;

        for(const auto &cat : categories)
        {
            FolderElement *foundFolder = currentFolder->folderWithName(cat);

            if(!foundFolder)
            {
                foundFolder = new FolderElement(cat);
                currentFolder->addChild(foundFolder);
            }

            currentFolder = foundFolder;
        }

        currentFolder->addChild(new NodeElement(t_data, dataList.length()));
        dataList.append(t_data);
    }

    bool showMenu(QPoint pos, DataType &selected)
    {
        QMenu menu;

        //QMenu *rootMenu = menu.addMenu("Add Effect");


        rootFolder->sortAlphabetically(true);

        std::function<void(QMenu &menu, NodeTreeElement *)> treeLoop;
        treeLoop = [&treeLoop](QMenu &menu, NodeTreeElement *rootElement){
            for(uint i = 0; i < rootElement->childCount(); ++i)
            {
                auto element = rootElement->elementAt(i);

                if(element->isFolder())
                {

                    treeLoop(*menu.addMenu(element->name()), element);
                }
                else
                {
                    auto action = menu.addAction(element->name());
                    action->setData(static_cast<NodeElement*>(element)->index());
                }
            }
        };

        treeLoop(menu, rootFolder);


        QAction *action = menu.exec(pos);

        if(action)
        {
            selected = dataList[action->data().toInt()];
            return true;
        }
        return false;
    }

private:
    FolderElement *rootFolder;
    QVector<DataType> dataList;
};

} // namespace photon

#endif // PHOTON_MENUFACTORY_H
