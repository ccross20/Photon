#ifndef SURFACEGRAPHWIDGET_H
#define SURFACEGRAPHWIDGET_H

#include <QWidget>
#include "surface/surfacegraph.h"

namespace photon {

class PHOTONCORE_EXPORT SurfaceGraphWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SurfaceGraphWidget(SurfaceGraph *graph, QWidget *parent = nullptr);
    ~SurfaceGraphWidget();

signals:

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // SURFACEGRAPHWIDGET_H
