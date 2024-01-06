#ifndef PIXELLAYOUTEDITOR_P_H
#define PIXELLAYOUTEDITOR_P_H

#include <QPushButton>
#include <QListWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QDoubleSpinBox>
#include "pixellayouteditor.h"
#include "pointedit.h"

namespace photon {

class PixelSourceItem;

class PixelLayoutPropertyEditor : public QWidget
{
    Q_OBJECT
public:
    PixelLayoutPropertyEditor();

public slots:
    void selectLayout(PixelSourceLayout*);
    void positionUpdated(const QPointF);
    void scaleUpdated(const QPointF);
    void rotationUpdated(double);

private:
    PixelSourceLayout *m_layout;
    PointEdit *m_positionEdit;
    PointEdit *m_scaleEdit;
    QDoubleSpinBox *m_rotationSpin;
};

class PixelLayoutScene : public QGraphicsScene
{
    Q_OBJECT
public:
    PixelLayoutScene(PixelLayout *);

    PixelSourceItem *findItemForSource(PixelSourceLayout *) const;
    void setScale(QPointF);
    QPointF scale() const{return m_scale;}
private slots:

    void sourceAdded(photon::PixelSourceLayout *);
    void sourceRemoved(photon::PixelSourceLayout *);
    void sourceTransformUpdated();

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    PixelLayout *m_layout;
    QVector<PixelSourceItem*> m_sourceItems;
    QPointF m_scale;
    QPointF m_inverseScale;

};

class PixelLayoutView : public QGraphicsView
{
public:
    PixelLayoutView();

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *) override;
};

class PixelSourceItem : public QGraphicsItem
{
public:
    PixelSourceItem(PixelSourceLayout *);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    PixelSourceLayout *sourceLayout() const{return m_layout;}

    QPointF scale() const{return m_scale;}
    void setScale(QPointF);
    void transformUpdated();
private:
    PixelSourceLayout *m_layout;
    QPointF m_scale;
    QPointF m_inverseScale;
};

class PixelLayoutEditorSidePanel : public QWidget
{
    Q_OBJECT
public:
    PixelLayoutEditorSidePanel(PixelLayout *);

public slots:
    void addSource(photon::PixelSource*);
    void addClicked();
    void selectedRow(int);

protected:

private:
    PixelLayout *pixelLayout;
    QPushButton *addButton;
    QListWidget *layoutList;
    PixelLayoutPropertyEditor *propertyEditor;
    PixelLayoutScene *scene;
    PixelLayoutView *view;
};

class PixelLayoutEditor::Impl
{
public:
    PixelLayout *pixelLayout;
    PixelLayoutEditorSidePanel *sidePanel;
};



}


#endif // PIXELLAYOUTEDITOR_P_H
