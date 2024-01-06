#ifndef PHOTON_TIMELINEVIEWER_H
#define PHOTON_TIMELINEVIEWER_H

#include <QGraphicsView>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT TimelineViewer : public QGraphicsView
{
    Q_OBJECT
public:
    TimelineViewer();
    ~TimelineViewer();


public slots:
    void movePlayheadTo(double t_time);
    void setScale(double);
    void setOffset(double);

signals:
    void scaleChanged(double);
    void offsetChanged(double);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void drawForeground(QPainter *painter, const QRectF &rect) override;
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void wheelEvent(QWheelEvent *event) override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_TIMELINEVIEWER_H
