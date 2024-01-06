#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QFormLayout>
#include <QDialogButtonBox>
#include "canvasdialog.h"
#include "pixel/canvas.h"

namespace photon {

class CanvasDialog::Impl{
public:
    Canvas *canvas;
    QSpinBox *widthSpin;
    QSpinBox *heightSpin;
    QLineEdit *nameEdit;
};


CanvasDialog::CanvasDialog(Canvas *t_canvas, QWidget *t_parent): QDialog(t_parent),m_impl(new Impl)
{
    m_impl->canvas = t_canvas;

    QFormLayout *form = new QFormLayout;

    m_impl->nameEdit = new QLineEdit;
    form->addRow("Name", m_impl->nameEdit);

    m_impl->widthSpin = new QSpinBox;
    m_impl->widthSpin->setRange(1,4096);
    form->addRow("Width", m_impl->widthSpin);

    m_impl->heightSpin = new QSpinBox;
    m_impl->heightSpin->setRange(1,4096);
    form->addRow("Height", m_impl->heightSpin);

    if(m_impl->canvas)
    {
        m_impl->nameEdit->setText(m_impl->canvas->name());
        m_impl->widthSpin->setValue(m_impl->canvas->width());
        m_impl->heightSpin->setValue(m_impl->canvas->height());
    }
    else
    {
        m_impl->nameEdit->setText("Canvas");
        m_impl->widthSpin->setValue(1024);
        m_impl->heightSpin->setValue(48);
    }


    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                         | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    form->addWidget(buttonBox);


    setLayout(form);
}

CanvasDialog::~CanvasDialog()
{
    delete m_impl;
}

void CanvasDialog::accept()
{
    if(!m_impl->canvas)
    {
        m_impl->canvas = new Canvas(m_impl->nameEdit->text(), QSize{m_impl->widthSpin->value(), m_impl->heightSpin->value()});
    }
    else
    {
        m_impl->canvas->setSize(QSize{m_impl->widthSpin->value(), m_impl->heightSpin->value()});
        m_impl->canvas->setName(m_impl->nameEdit->text());
    }

    QDialog::accept();
}

void CanvasDialog::reject()
{
    QDialog::reject();
}

Canvas *CanvasDialog::canvas() const
{
    return m_impl->canvas;
}

} // namespace photon
