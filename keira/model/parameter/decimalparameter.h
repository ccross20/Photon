#ifndef KEIRA_DECIMALPARAMETER_H
#define KEIRA_DECIMALPARAMETER_H
#include "parameter.h"

namespace keira {

class KEIRA_EXPORT DecimalParameter : public Parameter
{
public:

    const static QByteArray ParameterId;

    DecimalParameter();
    DecimalParameter(const QByteArray &t_id, const QString &t_name, double t_default, int connectionOptions = AllowMultipleOutput | AllowSingleInput);
    ~DecimalParameter();

    void setMinimum(double);
    void setMaximum(double);
    void setPrecision(uint);

    void setValue(const QVariant &) override;

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

#endif // KEIRA_DECIMALPARAMETER_H
