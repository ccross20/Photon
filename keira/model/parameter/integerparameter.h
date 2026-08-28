#ifndef KEIRA_INTEGERPARAMETER_H
#define KEIRA_INTEGERPARAMETER_H
#include "parameter.h"

namespace keira {

class KEIRA_EXPORT IntegerParameter : public Parameter
{
public:

    const static QByteArray ParameterId;

    IntegerParameter();
    IntegerParameter(const QByteArray &t_id, const QString &t_name, int t_default, int connectionOptions = AllowSingleInput);
    ~IntegerParameter();

    void setMinimum(int);
    void setMaximum(int);

    // Also accept a decimal source (its value is truncated/rounded on read)
    // and a boolean one (false = 0, true = 1).
    bool acceptsConnectionFrom(const Parameter *source) const override;

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

#endif // KEIRA_INTEGERPARAMETER_H
