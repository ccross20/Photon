#ifndef PHOTON_STACKEDPARAMETERWIDGET_H
#define PHOTON_STACKEDPARAMETERWIDGET_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT StackedParameterWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StackedParameterWidget(QWidget *parent = nullptr);
    ~StackedParameterWidget();

    void addWidget(QWidget *, const QString &name);

signals:

    void widgetResized(QSize);

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    class Impl;
    Impl *m_impl;

};

} // namespace photon

#endif // PHOTON_STACKEDPARAMETERWIDGET_H
