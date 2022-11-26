#ifndef KEIRA_STRINGPARAMETER_H
#define KEIRA_STRINGPARAMETER_H
#include "parameter.h"

namespace keira {

class KEIRA_EXPORT StringParameter : public Parameter
{
public:

    const static QByteArray ParameterId;

    StringParameter();
    StringParameter(const QByteArray &t_id, const QString &t_name, QString t_default, int connectionOptions = AllowMultipleOutput | AllowSingleInput);
    ~StringParameter();

    void setMaxLength(int);

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

#endif // KEIRA_STRINGPARAMETER_H
