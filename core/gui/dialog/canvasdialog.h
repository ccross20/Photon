#ifndef PHOTON_CANVASDIALOG_H
#define PHOTON_CANVASDIALOG_H
#include <QDialog>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT CanvasDialog : public QDialog
{
    Q_OBJECT
public:
    CanvasDialog(Canvas *canvas = nullptr, QWidget *parent = nullptr);
    ~CanvasDialog();

    Canvas *canvas() const;

public slots:
    void accept() override;
    void reject() override;



private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_CANVASDIALOG_H
