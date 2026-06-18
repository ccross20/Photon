#ifndef NODEEDITOR_H
#define NODEEDITOR_H

#include "keira-global.h"
#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>

namespace keira {

class Node;

class KEIRA_EXPORT NodeEditor : public QWidget
{
    Q_OBJECT
public:
    explicit NodeEditor(QWidget *parent = nullptr);

    void setNode(Node *t_node);
    Node *node() const;

signals:

public slots:
    void widgetUpdated(QWidget *, const keira::Parameter *);

private:
    void rebuildParameters();
    void removeAllFromLayout(QLayout *layout);

    Node *m_node = nullptr;
    QGridLayout *m_gridLayout = nullptr;
    QVBoxLayout *m_vLayout = nullptr;

};

} // namespace keira

#endif // NODEEDITOR_H
