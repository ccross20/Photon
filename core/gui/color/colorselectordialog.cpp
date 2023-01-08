#include "colorselectordialog.h"


namespace photon {

ColorSelectorDialog::ColorSelectorDialog(const QColor &color, QWidget *parent): QDialog (parent),m_selectorWidget(new ColorSelectorWidget{color})
{
    m_ogColor = color;

    setMinimumSize(400,200);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->addWidget(m_selectorWidget);

    setLayout(vLayout);

    connect(m_selectorWidget,SIGNAL(selectionChanged(QColor)),this,SIGNAL(selectionChanged(QColor)));
   // connect(this,SIGNAL(rejected()),this,SLOT(dialogCancelled()));

}

void ColorSelectorDialog::dialogCancelled()
{
    emit selectionChanged(m_ogColor);
}

} // namespace exo
