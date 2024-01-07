#ifndef PHOTON_TIMELINEVIEWER_H
#define PHOTON_TIMELINEVIEWER_H

#include <QGraphicsView>
#include "photon-global.h"

namespace photon {

class TimelineScene;

class PHOTONCORE_EXPORT TimelineViewer : public QWidget
{
    Q_OBJECT
public:
    TimelineViewer();
    ~TimelineViewer();

    QPointF mapToScene(const QPointF&);

public slots:
    void movePlayheadTo(double t_time);
    void setScale(double);
    void setOffset(double);
    void setScene(TimelineScene *);

signals:
    void scaleChanged(double);
    void offsetChanged(double);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect);
    void drawForeground(QPainter *painter, const QRectF &rect);
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;
    void setTransform(const QTransform &);

private:
    QGraphicsSceneMouseEvent *createMouseEvent(QMouseEvent *);

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TIMELINEVIEWER_H
