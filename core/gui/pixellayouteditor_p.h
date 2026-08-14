#ifndef PIXELLAYOUTEDITOR_P_H
#define PIXELLAYOUTEDITOR_P_H

#include <QPushButton>
#include <QListWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <functional>
#include "pixellayouteditor.h"

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;

namespace photon {

class PixelPointItem;
class SceneObject;

// A QListWidget that also accepts a Fixture/PixelSource-capable SceneObject
// dropped from the Project panel (or anywhere else SceneObjectMime is
// dragged from), re-emitting the decoded objects for the side panel to
// filter and add.
class PixelSourceListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit PixelSourceListWidget(QWidget *parent = nullptr);

signals:
    void sceneObjectsDropped(const QVector<SceneObject*> &objects);

protected:
    void dragEnterEvent(QDragEnterEvent *) override;
    void dragMoveEvent(QDragMoveEvent *) override;
    void dropEvent(QDropEvent *) override;
};

class PixelLayoutScene : public QGraphicsScene
{
    Q_OBJECT
public:
    PixelLayoutScene(PixelLayout *);

    // t_scale is always uniform (x==y) - the edit area is a fixed square
    // regardless of the panel's own aspect ratio; t_offset positions that
    // square's origin within the panel so it's centered rather than
    // anchored to a corner, letterboxed on whichever axis is longer.
    // Computed and applied together (not via the view's own transform/scroll
    // - QGraphicsView's translate()/centerOn() didn't reliably move anything
    // here once the scene rect already matched the viewport size 1:1) so
    // point positioning stays simple, explicit scene-space arithmetic we
    // control end to end.
    void setViewport(QPointF scale, QPointF offset);
    QPointF scale() const{return m_scale;}
    QPointF offset() const{return m_offset;}

    // Marks t_source (or none, for nullptr) as the active source, so its
    // points paint cyan instead of dark grey - kept in sync with the fixture
    // list's current row. Independent of native Qt item selection (below).
    void selectSource(PixelSourceLayout *t_source);
    PixelSourceLayout *activeSourceLayout() const{return m_activeSourceLayout;}

    // Every currently-selected point (native QGraphicsItem selection, e.g.
    // from a rubber-band drag - independent of which source is "active"),
    // as (owning layout, pixel index) pairs. Deterministically ordered by
    // m_layout->sourceLayouts() order then ascending index - selectedItems()
    // itself has no stable order, and shape commands like Grid need one.
    QVector<QPair<PixelSourceLayout*,int>> selectedPixels() const;

private slots:
    void sourceAdded(photon::PixelSourceLayout *);
    void sourceRemoved(photon::PixelSourceLayout *);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    void repositionSource(PixelSourceLayout *);
    void syncPointCount(PixelSourceLayout *);

    PixelLayout *m_layout;
    QHash<PixelSourceLayout*, QVector<PixelPointItem*>> m_pointItems;
    PixelSourceLayout *m_activeSourceLayout = nullptr;
    QPointF m_scale;
    QPointF m_offset;
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

// One point per pixel (replaces the old one-item-per-source PixelSourceItem)
// so individual pixels can be natively selected/dragged via QGraphicsScene's
// built-in rubber-band selection.
class PixelPointItem : public QGraphicsItem
{
public:
    PixelPointItem(PixelSourceLayout *, int index);
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
               QWidget *widget) override;

    PixelSourceLayout *sourceLayout() const{return m_sourceLayout;}
    int index() const{return m_index;}

    // Recomputes this point's scene position from
    // sourceLayout->pixelPosition(index) and the scene's current pixel
    // scale. Called on scene rescale, on pixelPositionsChanged, and after
    // syncPointCount() adds new points.
    void reposition();

    // Only the active source's points are selectable/draggable - kept in
    // sync with PixelLayoutScene::selectSource(). Clears any existing
    // selection when turned off, since a point that can no longer be
    // selected shouldn't linger in a stale selected state.
    void setInteractive(bool);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    PixelSourceLayout *m_sourceLayout;
    int m_index;
    bool m_repositioning = false;
};

class PixelLayoutEditorSidePanel : public QWidget
{
    Q_OBJECT
public:
    PixelLayoutEditorSidePanel(PixelLayout *);

public slots:
    void addSource(photon::PixelSource*);
    void addClicked();
    void removeClicked();
    void arrangeClicked();
    void selectedRow(int);
    void sceneObjectsDropped(const QVector<SceneObject*> &objects);

protected:
    bool isAlreadyAdded(SceneObject *) const;
    void tryAddSceneObject(SceneObject *);

    void arrangeLinear();
    void arrangeGrid();
    void arrangeRadial();
    void arrangeArc();
    void arrangeHoneycomb();
    void arrangeBeeEye();
    void arrangeMove();
    void arrangeScale();
    void arrangeRotate();

    // Recenters t_shapePoints (in the shape function's own canonical local
    // convention) around the current selection's canvas-space bounding-box
    // centroid, then writes each one back into its point's own source layout.
    void commitArrangedPoints(const QVector<QPointF> &shapePoints);

    // Applies fn to each selected pixel's current position independently
    // (as opposed to commitArrangedPoints(), which replaces the whole
    // selection with a freshly-generated shape) - used by Move/Scale/Rotate.
    void transformSelected(const std::function<QPointF(QPointF)> &fn);

private:
    PixelLayout *pixelLayout;
    QPushButton *addButton;
    QPushButton *removeButton;
    QPushButton *arrangeButton;
    PixelSourceListWidget *layoutList;
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
