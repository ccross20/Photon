#ifndef KEIRA_OPTIONPARAMETER_H
#define KEIRA_OPTIONPARAMETER_H

#include "parameter.h"

namespace keira {

class KEIRA_EXPORT OptionParameter : public Parameter
{
public:
    const static QByteArray ParameterId;

    OptionParameter();
    OptionParameter(const QByteArray &t_id, const QString &t_name, const QStringList &t_options, int t_default, int connectionOptions = AllowMultipleOutput | AllowSingleInput);
    ~OptionParameter();

    void setOptions(const QStringList &);
    const QStringList &options() const;

    QWidget *createWidget(NodeItem *) const override;
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
