#ifndef PHOTON_DMXVIEWER_H
#define PHOTON_DMXVIEWER_H

#include <QWidget>

namespace photon {

class DMXViewer : public QWidget
{
    Q_OBJECT
public:
    explicit DMXViewer(QWidget *parent = nullptr);
    ~DMXViewer();

signals:

private slots:
    void timeout();

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_DMXVIEWER_H
