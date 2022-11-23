#ifndef KEIRA_VIEWER_H
#define KEIRA_VIEWER_H

#include <QGraphicsView>
#include "keira-global.h"

namespace keira {

class KEIRA_EXPORT Viewer : public QGraphicsView
{
    Q_OBJECT
public:
    explicit Viewer(QWidget *parent = nullptr);
    virtual ~Viewer();

    Graph *graph() const;
    void zoom(double value);
    void zoom(double value, QPointF pt);

signals:
    void zoomChanged(double value);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;


private:
    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // KEIRA_VIEWER_H
