#include <QComboBox>
#include <QLabel>
#include "optionparameter.h"
#include "view/nodeitem.h"

namespace keira {

const QByteArray OptionParameter::ParameterId = "option";

class OptionParameter::Impl
{
public:
    QStringList options;
};

OptionParameter::OptionParameter() : Parameter(),m_impl(new Impl)
{

}

OptionParameter::OptionParameter(const QByteArray &t_id, const QString &t_name, const QStringList &t_options, int t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, t_default, connectionOptions),m_impl(new Impl)
{
    m_impl->options = t_options;
}

OptionParameter::~OptionParameter()
{
    delete m_impl;
}


void OptionParameter::setOptions(const QStringList &t_options)
{
    m_impl->options = t_options;
}

const QStringList &OptionParameter::options() const
{
    return m_impl->options;
}

QWidget *OptionParameter::createWidget(NodeItem *item) const
{
    if(isReadOnly())
    {
        QLabel *label = new QLabel();
        label->setMaximumHeight(30);
        label->setStyleSheet("background:transparent;");
        label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
        return label;
    }
    QComboBox *combo = new QComboBox();
    combo->addItems(m_impl->options);
    combo->setMaximumHeight(30);
    combo->setMinimumWidth(50);

    combo->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));


    const OptionParameter *param = this;
    QComboBox::connect(combo, &QComboBox::currentIndexChanged, combo,[item, combo, param](){item->widgetUpdated(combo, param);});
    return combo;
}

void OptionParameter::updateWidget(QWidget *t_widget) const
{
    if(isReadOnly())
    {
        QLabel *label = static_cast<QLabel*>(t_widget);
        label->setText(value().toString());
    }
    else
    {
        QComboBox *spinBox = static_cast<QComboBox*>(t_widget);
        spinBox->setCurrentIndex(value().toInt());
    }

}

QVariant OptionParameter::updateValue(QWidget *t_widget) const
{
    if(isReadOnly())
        return static_cast<QLabel*>(t_widget)->text();
    return static_cast<QComboBox*>(t_widget)->currentIndex();
}

void OptionParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

    m_impl->options.clear();
    if(t_json.contains("options"))
    {
        auto optionArray = t_json.value("options").toArray();
        for(const auto &option : optionArray)
        {
            m_impl->options.append(option.toString());
        }
    }

}

void OptionParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

    QJsonArray optionArray;
    for(const QString &option : m_impl->options)
    {
        optionArray.append(option);
    }
    t_json.insert("options",optionArray);
}

} // namespace photon
