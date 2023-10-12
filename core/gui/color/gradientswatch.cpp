#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include "vektor/vektorartist.h"
#include "gradientswatch.h"
#include "gradientdialog.h"
#include "exocore.h"
#include "gui/guimanager.h"
#include "settings/settings.h"
#include "vektor/vektorrenderer.h"

namespace exo {

class GradientSwatch::Impl
{
public:
    Impl(const Gradient &gradient);
    void renderGradient(const QSize &size);

    Gradient gradient;
    int maxWidth = 100;
    QImage gradientImage;
};


GradientSwatch::Impl::Impl(const Gradient &_gradient) : gradient(_gradient)
{

}

void GradientSwatch::Impl::renderGradient(const QSize &size)
{

    VektorRenderer m_renderer;
    Vektor m_gradientBarVektor;
    VektorArtist builder(&m_gradientBarVektor);
    builder.drawRect(0,0,size.width(), size.height());
    builder.endVektor();

    //m_gradientBarVektor.setFillTransform(gradient_affine(0, 0, size.width(), 0, 1).inverted());

    gradientImage = QImage{size,QImage::Format_ARGB32};
    gradientImage.fill(Qt::green);
    QPainter painter{&gradientImage};
    painter.fillRect(gradientImage.rect(), exoApp->settings()->alphaBackgroundBrush());

    Gradient g = gradient;
    g.type = Gradient::TypeLinear;
    g.overshoot = Gradient::OverShootClamp;
    g.angle = 0.0;

    rendering_buffer buffer(gradientImage.bits(), static_cast<uint>(gradientImage.width()),size.height(),gradientImage.bytesPerLine());
    m_renderer.render(buffer,&m_gradientBarVektor,trans_affine{});

}

GradientSwatch::GradientSwatch(const Gradient &gradient, QWidget *parent) : QWidget(parent), m_impl(new Impl(gradient))
{
    setMinimumSize(50,24);
}

GradientSwatch::~GradientSwatch()
{
    delete m_impl;
}

const Gradient &GradientSwatch::gradient() const
{
    return m_impl->gradient;
}

void GradientSwatch::setGradient(const Gradient &gradient)
{
    if(m_impl->gradient == gradient)
        return;
    m_impl->gradient = gradient;

    m_impl->renderGradient(size());

    //if(color.format == Color::ColorFormatCMYKA)
        //qDebug() << color;
    emit gradientChanged(gradient);
    update();
}

void GradientSwatch::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter p(this);
    p.drawImage(QPoint(0,0), m_impl->gradientImage);

}

void GradientSwatch::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_impl->renderGradient(event->size());
}

void GradientSwatch::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    emit beganEditing();
    emit swatchClicked();

    GradientDialog *colorsWidget = new GradientDialog(m_impl->gradient, window());
    colorsWidget->setAttribute(Qt::WA_DeleteOnClose);
    colorsWidget->show();
    colorsWidget->raise();
    colorsWidget->activateWindow();
    connect(colorsWidget,SIGNAL(selectionChanged(Gradient)),this,SLOT(setGradient(Gradient)));
    connect(colorsWidget,SIGNAL(dialogAccepted()),this,SIGNAL(finishedEditing()));
    connect(colorsWidget,SIGNAL(dialogRejected()),this,SIGNAL(finishedEditing()));
}

void GradientSwatch::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void GradientSwatch::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}


} // namespace exo
