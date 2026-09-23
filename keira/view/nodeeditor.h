#ifndef NODEEDITOR_H
#define NODEEDITOR_H

#include "keira-global.h"
#include <QWidget>

class QVBoxLayout;

namespace photon { class PropertyForm; }

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

    Node *m_node = nullptr;
    QVBoxLayout *m_vLayout = nullptr;
    photon::PropertyForm *m_form = nullptr;

};

} // namespace keira

#endif // NODEEDITOR_H
