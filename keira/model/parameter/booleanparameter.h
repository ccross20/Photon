#ifndef BOOLEANPARAMETER_H
#define BOOLEANPARAMETER_H
#include "parameter.h"

namespace keira {

class KEIRA_EXPORT BooleanParameter : public Parameter
{
public:

    const static QByteArray ParameterId;

    BooleanParameter();
    BooleanParameter(const QByteArray &t_id, const QString &t_name, bool t_default, int connectionOptions = AllowMultipleOutput | AllowSingleInput);
    ~BooleanParameter();

    QWidget *createWidget(NodeEditor *) const override;
    void updateWidget(QWidget *) const override;
    QVariant updateValue(QWidget *) const override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;
};

} // namespace keira

#endif // BOOLEANPARAMETER_H
