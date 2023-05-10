#include <QDoubleSpinBox>
#include <QLabel>
#include "vector3dparameter.h"

namespace photon {

const QByteArray Vector3DParameter::ParameterId = "vector3D";

Vector3DParameter::Vector3DParameter()
{

}

Vector3DParameter::Vector3DParameter(const QByteArray &t_id, const QString &t_name, QVector3D t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, t_default, connectionOptions)
{

}


QWidget *Vector3DParameter::createWidget(keira::NodeItem *item) const
{
    if(isReadOnly())
    {
        QLabel *label = new QLabel();
        label->setMaximumHeight(30);
        label->setStyleSheet("background:transparent;");
        label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
        return label;
    }

    //const Vector3DParameter *param = this;


    QLabel *label = new QLabel();
    label->setMaximumHeight(30);
    label->setStyleSheet("background:transparent;");
    label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
    return label;
}

void Vector3DParameter::updateWidget(QWidget *t_widget) const
{
    if(isReadOnly())
    {
        QLabel *label = static_cast<QLabel*>(t_widget);
        label->setText(value().toString());
    }
    else
    {
        QLabel *label = static_cast<QLabel*>(t_widget);
        label->setText(value().toString());
    }

}

QVariant Vector3DParameter::updateValue(QWidget *t_widget) const
{
    if(isReadOnly())
        return static_cast<QLabel*>(t_widget)->text();
    return static_cast<QLabel*>(t_widget)->text();
}

void Vector3DParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

}

void Vector3DParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

}

} // namespace photon
