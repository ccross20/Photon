#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "keira-global.h"

namespace keira {

class Scene;
class Viewer;
class NodeEditor;

class KEIRA_EXPORT GraphWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GraphWidget(NodeLibrary *t_library, QWidget *parent = nullptr);

    void setScene(Scene *t_scene);
    Scene *scene() const;

signals:

private slots:
    void selectionUpdated();
    void subGraphOpened(Graph *);
    void graphUpdated(Graph *);
    void gotoParentGraph();

private:
    Scene *m_scene = nullptr;
    Viewer *m_viewer = nullptr;
    NodeEditor *m_editor = nullptr;
    QLabel *m_navigationLabel;
    QPushButton *m_upButton;
};

} // namespace keira

#endif // GRAPHWIDGET_H
