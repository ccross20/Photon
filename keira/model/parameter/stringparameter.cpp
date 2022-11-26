#include <QLineEdit>
#include <QLabel>
#include "stringparameter.h"
#include "view/nodeitem.h"

namespace keira {

const QByteArray StringParameter::ParameterId = "string";

class StringParameter::Impl
{
public:
    int maxLength = 24;
};

StringParameter::StringParameter() : Parameter(),m_impl(new Impl)
{

}

StringParameter::StringParameter(const QByteArray &t_id, const QString &t_name, QString t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, t_default, connectionOptions),m_impl(new Impl)
{

}

StringParameter::~StringParameter()
{
    delete m_impl;
}

void StringParameter::setMaxLength(int t_min)
{
    m_impl->maxLength = t_min;
}

QWidget *StringParameter::createWidget(NodeItem *item) const
{

    QLineEdit *lineEdit = new QLineEdit();
    lineEdit->setMaximumHeight(30);
    lineEdit->setMaxLength(m_impl->maxLength);
    lineEdit->setMinimumWidth(50);

    lineEdit->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));

    //lineEdit->setReadOnly(isReadOnly());

    const StringParameter *param = this;
    QLineEdit::connect(lineEdit, &QLineEdit::editingFinished, lineEdit,[item, lineEdit, param](){item->widgetUpdated(lineEdit, param);});
    return lineEdit;
}

void StringParameter::updateWidget(QWidget *t_widget) const
{

    QLineEdit *lineEdit = static_cast<QLineEdit*>(t_widget);
    lineEdit->setText(value().toString());


}

QVariant StringParameter::updateValue(QWidget *t_widget) const
{
    return static_cast<QLineEdit*>(t_widget)->text();
}

void StringParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

    m_impl->maxLength = t_json.value("max-length").toInt();
}

void StringParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

    t_json.insert("max-length", m_impl->maxLength);
}

} // namespace keira

