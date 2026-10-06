#ifndef KEIRA_OPTIONPARAMETER_H
#define KEIRA_OPTIONPARAMETER_H

#include "parameter.h"

namespace keira {

class KEIRA_EXPORT OptionParameter : public Parameter
{
public:
    const static QByteArray ParameterId;

    OptionParameter();
    OptionParameter(const QByteArray &t_id, const QString &t_name, const QStringList &t_options, int t_default, int connectionOptions =  AllowSingleInput);
    ~OptionParameter();

    void setOptions(const QStringList &);
    QStringList options() const;

    // Numbers, integers and booleans can drive an option: the value is taken
    // as the option's index.
    bool acceptsConnectionFrom(const Parameter *source) const override;
    // Stores a whole index: a number is floored (2.7 picks option 2), and
    // anything past either end is clamped to the first/last option.
    void setValue(const QVariant &) override;

    QWidget *createWidget(NodeEditor *) const override;
    void updateWidget(QWidget *) const override;
    QVariant updateValue(QWidget *) const override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // KEIRA_OPTIONPARAMETER_H
