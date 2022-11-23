#ifndef PHOTON_DMXVIEWERCELLS_H
#define PHOTON_DMXVIEWERCELLS_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

class DMXViewerCells : public QWidget
{
    Q_OBJECT
public:
    explicit DMXViewerCells(QWidget *parent = nullptr);
    ~DMXViewerCells();

signals:

public slots:
    void setData(const photon::DMXMatrix &);

protected:
    void leaveEvent(QEvent *) override;
    void paintEvent(QPaintEvent *e) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_DMXVIEWERCELLS_H
