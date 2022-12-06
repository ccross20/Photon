#include <QVBoxLayout>
#include <QLabel>
#include <QResizeEvent>
#include "stackedparameterwidget.h"


namespace photon {

class StackedParameterWidget::Impl
{
public:
    QVBoxLayout *layout;
};

StackedParameterWidget::StackedParameterWidget(QWidget *parent)
    : QWidget{parent},m_impl(new Impl)
{
    m_impl->layout = new QVBoxLayout;
    m_impl->layout->setContentsMargins(0,0,0,0);
    setAttribute(Qt::WidgetAttribute::WA_TranslucentBackground);
    setLayout(m_impl->layout);
}

StackedParameterWidget::~StackedParameterWidget()
{
    delete m_impl;
}

void StackedParameterWidget::addWidget(QWidget *t_widget, const QString &t_name)
{
    m_impl->layout->addWidget(new QLabel(t_name));
    m_impl->layout->addWidget(t_widget);
}

void StackedParameterWidget::resizeEvent(QResizeEvent *t_event)
{
    QWidget::resizeEvent(t_event);

    emit widgetResized(t_event->size());
}

} // namespace photon
