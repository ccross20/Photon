#ifndef DECO_GRADIENTDIALOG_H
#define DECO_GRADIENTDIALOG_H

#include "gui/dialog/dialogwindow.h"
#include "gradienteditor.h"

namespace exo {

class EXOCORE_EXPORT GradientDialog : public DialogWindow
{
    Q_OBJECT
public:
    GradientDialog(const Gradient &gradient = Gradient{}, QWidget *parent = nullptr);

signals:
    void selectionChanged(const Gradient &gradient);

private slots:
    void dialogCancelled();
    void resizeContent();

private:
    GradientEditor *m_editorWidget;
    Gradient m_ogGradient;
};

} // namespace exo

#endif // DECO_GRADIENTDIALOG_H
