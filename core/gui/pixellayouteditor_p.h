#ifndef PIXELLAYOUTEDITOR_P_H
#define PIXELLAYOUTEDITOR_P_H

#include <QPushButton>
#include <QListWidget>
#include "pixellayouteditor.h"

namespace photon {

class PixelLayoutEditorSidePanel : public QWidget
{
public:
    PixelLayoutEditorSidePanel(PixelLayout *);

public slots:
    void addSource(photon::PixelSource*);
    void addClicked();

private:
    PixelLayout *pixelLayout;
    QPushButton *addButton;
    QListWidget *layoutList;
};

class PixelLayoutEditor::Impl
{
public:
    PixelLayout *pixelLayout;
    PixelLayoutEditorSidePanel *sidePanel;
};



}


#endif // PIXELLAYOUTEDITOR_P_H
