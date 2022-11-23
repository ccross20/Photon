#ifndef KEIRA_PLAYGROUNDWIDGET_H
#define KEIRA_PLAYGROUNDWIDGET_H

#include <QWidget>

namespace keira {

class PlaygroundWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PlaygroundWidget(QWidget *parent = nullptr);

signals:

};

} // namespace keira

#endif // KEIRA_PLAYGROUNDWIDGET_H
