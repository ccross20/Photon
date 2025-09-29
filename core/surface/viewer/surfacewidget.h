#ifndef SURFACEWIDGET_H
#define SURFACEWIDGET_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT SurfaceWidget : public QWidget
{
    Q_OBJECT
public:


    explicit SurfaceWidget(QWidget *parent = nullptr);
    ~SurfaceWidget();

    void setSurface(Surface *);
    Surface *surface() const;

    SurfaceMode mode() const;

signals:
    void modeChanged(SurfaceMode mode);

public slots:
    void setMode(SurfaceMode);
    void rebuildLayout();

protected:
    void showEvent(QShowEvent*) override;
    void resizeEvent(QResizeEvent*) override;

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // SURFACEWIDGET_H
