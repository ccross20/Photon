#include <QPushButton>
#include "buttonparameter.h"
#include "model/node.h"

namespace keira {

const QByteArray ButtonParameter::ParameterId = "button";

ButtonParameter::ButtonParameter()
{

}

ButtonParameter::ButtonParameter(const QByteArray &t_id, const QString &t_name) : Parameter(ParameterId, t_id, t_name, 0, NoConnection)
{

}

QWidget *ButtonParameter::createWidget(NodeItem *item) const
{
    QPushButton *button = new QPushButton(name());
    const ButtonParameter *param = this;
    QPushButton::connect(button, &QPushButton::clicked, button,[param](){param->node()->buttonClicked(param);});
    return button;
}

} // namespace photon
