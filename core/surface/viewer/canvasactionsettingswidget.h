#ifndef CANVASACTIONSETTINGSWIDGET_H
#define CANVASACTIONSETTINGSWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include "surface/canvasaction.h"

namespace photon {

class CanvasActionSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CanvasActionSettingsWidget(CanvasAction *t_action, QWidget *parent = nullptr);


public slots:
    void canvasChanged(int);
    void openAddPixelLayout();
    void removeSelectedLayout();
    void pixelLayoutAdded(photon::PixelLayout *);
    void pixelLayoutRemoved(photon::PixelLayout *);

private:
    CanvasAction *m_canvasAction;
    QListWidget *m_pixelLayoutList;
    QPushButton *m_addLayoutButton;
    QPushButton *m_removeLayoutButton;
    QComboBox *m_canvasCombo;
};

} // namespace photon

#endif // CANVASACTIONSETTINGSWIDGET_H
