#ifndef CHANNELEFFECTEDITOR_P_H
#define CHANNELEFFECTEDITOR_P_H

#include <QSettings>
#include <QGraphicsView>
#include <QPainterPath>
#include <QGraphicsWidget>
#include <QGraphicsProxyWidget>
#include "sequence/channeleffect.h"
#include "channeleffecteditor.h"

namespace photon {

class EffectEditorScene : public QGraphicsScene
{

};


class WidgetContainer : public QGraphicsWidget
{
    Q_OBJECT
public:
    WidgetContainer(QWidget *, const QString &title);

    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void addedToScene(QGraphicsScene *);
    bool isCustomLayout() const{return m_customLayout;}

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override
    {
        QGraphicsWidget::mouseMoveEvent(event);
        m_customLayout = true;
    }

private slots:
    void sizeUpdated(QSize);

private:
    int m_inset = 5;
    bool m_customLayout = false;
    QWidget *m_widget;
    QString m_title;
    QGraphicsProxyWidget *m_proxy;


};

class EffectEditorViewer : public QGraphicsView
{
    Q_OBJECT
public:
    struct EffectViewerState
    {
        double offset;
        double scale;
    };



    EffectEditorViewer(ChannelEffect *);

    void setOffset(double);
    void setScale(QPointF);
    void remakeTransform();
    QTransform transform() const;
    QRectF sceneBounds() const;
    QPointF viewScale() const;
    QPointF viewOffset() const;

signals:
    void relayout();
    void scaleChanged(QPointF);
    void xScaleChanged(double);
    void offsetChanged(double);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

private:
    void rebuildPaths();

    QHash<QByteArray, EffectViewerState> m_states;
    QPainterPath m_path;
    QPainterPath m_channelPath;
    QRectF m_sceneBounds;
    QTransform m_transform;
    ChannelEffect *m_effect;
    QPoint m_startPt;
    QPoint m_lastPt;
    double m_xScale = 1;
    double m_yScale = 50;
    double m_xOffset = 0;
    double m_yOffset = 200;
    double m_startXPos;
    bool m_pathsDirty = true;
};


class ChannelEffectEditor::Impl
{
public:

    QVector<WidgetContainer*> containers;
    EffectEditorViewer *viewer;
    EffectEditorScene *scene;
    ChannelEffect *effect;
    QToolBar *toolbar;
    QRectF sceneBounds;
    QPointF scale;
    double offset;

};

}


#endif // CHANNELEFFECTEDITOR_P_H
