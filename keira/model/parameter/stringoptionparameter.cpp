#include <QComboBox>
#include <QLabel>
#include "stringoptionparameter.h"
#include "view/nodeeditor.h"

namespace keira {

const QByteArray StringOptionParameter::ParameterId = "stringOption";

class StringOptionParameter::Impl
{
public:
    QStringList options;
    OptionLambda optionLambda;
};

StringOptionParameter::StringOptionParameter() : Parameter(),m_impl(new Impl)
{

}

StringOptionParameter::StringOptionParameter(const QByteArray &t_id, const QString &t_name, const QStringList &t_options, int t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, t_default, connectionOptions),m_impl(new Impl)
{
    m_impl->options = t_options;
}

StringOptionParameter::~StringOptionParameter()
{
    delete m_impl;
}


void StringOptionParameter::setOptions(const QStringList &t_options)
{
    m_impl->options = t_options;
}

QStringList StringOptionParameter::options() const
{
    return m_impl->options;
}


void StringOptionParameter::setOptionLambda(OptionLambda t_lambda)
{
    m_impl->optionLambda = t_lambda;

    if(value().toInt() == 0)
    {
        auto options = m_impl->optionLambda();

        if(!options.isEmpty())
        {
            setValue(options.first().second);
            qDebug() << "Set value" << options.first().second;
        }
    }
}

OptionLambda StringOptionParameter::optionLambda() const
{
    return m_impl->optionLambda;
}

QWidget *StringOptionParameter::createWidget(NodeEditor *item) const
{
    if(isReadOnly())
    {
        QLabel *label = new QLabel();
        label->setText(value().toString());
        label->setMaximumHeight(30);
        label->setStyleSheet("background:transparent;");
        label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
        return label;
    }
    QComboBox *combo = new QComboBox();

    if(m_impl->optionLambda)
    {
        auto options = m_impl->optionLambda();
        for(auto option : options)
        {
            combo->addItem(option.first, option.second);
        }
    }
    else
        combo->addItems(m_impl->options);
    combo->setMaximumHeight(30);
    combo->setMinimumWidth(50);

    combo->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Minimum));

    // Initialise the selection to the current value. The node editor builds widgets
    // via createWidget only (it never calls updateWidget), so without this the combo
    // always shows the first item regardless of the stored value. Done before the
    // signal connection so it doesn't fire a spurious change.
    if(m_impl->optionLambda)
    {
        const int idx = combo->findData(value());
        if(idx >= 0)
            combo->setCurrentIndex(idx);
    }
    else
        combo->setCurrentText(value().toString());

    const StringOptionParameter *param = this;
    QComboBox::connect(combo, &QComboBox::currentIndexChanged, combo,[item, combo, param](){item->widgetUpdated(combo, param);});
    return combo;
}

void StringOptionParameter::updateWidget(QWidget *t_widget) const
{
    if(isReadOnly())
    {
        QLabel *label = static_cast<QLabel*>(t_widget);
        label->setText(value().toString());
    }
    else
    {
        QComboBox *combo = static_cast<QComboBox*>(t_widget);
        combo->setCurrentText(value().toString());
    }

}

QVariant StringOptionParameter::updateValue(QWidget *t_widget) const
{
    if(isReadOnly())
        return static_cast<QLabel*>(t_widget)->text();
    if(m_impl->optionLambda)
        return static_cast<QComboBox*>(t_widget)->currentData();
    else
        return static_cast<QComboBox*>(t_widget)->currentText();
}

void StringOptionParameter::readFromJson(const QJsonObject &t_json)
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

    setValue(t_json.value("value").toString());

}

void StringOptionParameter::writeToJson(QJsonObject &t_json) const
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
