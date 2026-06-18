#ifndef STRINGOPTIONPARAMETER_H
#define STRINGOPTIONPARAMETER_H


#include "parameter.h"

namespace keira {

using OptionLambda = std::function<QVector<std::pair<QString,QString>>()>;

class KEIRA_EXPORT StringOptionParameter : public Parameter
{
public:
    const static QByteArray ParameterId;

    StringOptionParameter();
    StringOptionParameter(const QByteArray &t_id, const QString &t_name, const QStringList &t_options, int t_default, int connectionOptions = AllowMultipleOutput | AllowSingleInput);
    ~StringOptionParameter();

    void setOptionLambda(OptionLambda lambda);
    OptionLambda optionLambda() const;
    void setOptions(const QStringList &);
    QStringList options() const;

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

#endif // STRINGOPTIONPARAMETER_H
