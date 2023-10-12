#include <QVBoxLayout>
#include "gradientdialog.h"

namespace exo {

GradientDialog::GradientDialog(const Gradient &gradient, QWidget *parent): DialogWindow (parent),m_editorWidget(new GradientEditor{gradient})
{
    setWindowTitle("Edit Gradient");
    setMenubarVisible(true);
    m_ogGradient = gradient;

    setMinimumSize(400,300);


    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(m_editorWidget);

    setContentLayout(vLayout);

    connect(m_editorWidget,SIGNAL(gradientChanged(Gradient)),this,SIGNAL(selectionChanged(Gradient)));
    connect(m_editorWidget,SIGNAL(resized()),this,SLOT(resizeContent()));
    connect(this,SIGNAL(rejected()),this,SLOT(dialogCancelled()));

}

void GradientDialog::dialogCancelled()
{
    emit selectionChanged(m_ogGradient);
}

void GradientDialog::resizeContent()
{
    adjustSize();
}

} // namespace exo
