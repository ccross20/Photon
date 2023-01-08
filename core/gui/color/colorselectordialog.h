#ifndef COLORSELECTORDIALOG_H
#define COLORSELECTORDIALOG_H

#include <QDialog>
#include "colorselectorwidget.h"

namespace photon {

class PHOTONCORE_EXPORT ColorSelectorDialog : public QDialog
{
    Q_OBJECT
public:
    ColorSelectorDialog(const QColor &color = QColor(Qt::red), QWidget *parent = nullptr);

signals:
    void selectionChanged(QColor color);

private slots:
    void dialogCancelled();

private:
    ColorSelectorWidget *m_selectorWidget;
    QColor m_ogColor;
};

} // namespace exo

#endif // COLORSELECTORDIALOG_H
