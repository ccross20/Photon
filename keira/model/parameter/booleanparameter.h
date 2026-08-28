#ifndef BOOLEANPARAMETER_H
#define BOOLEANPARAMETER_H
#include "parameter.h"

namespace keira {

class KEIRA_EXPORT BooleanParameter : public Parameter
{
public:

    const static QByteArray ParameterId;

    BooleanParameter();
    BooleanParameter(const QByteArray &t_id, const QString &t_name, bool t_default, int connectionOptions = AllowSingleInput);
    ~BooleanParameter();

    // Numbers convert on the way in: anything greater than 0 is true.
    void setValue(const QVariant &) override;

    // Also accept integer and decimal sources, using that same rule.
    bool acceptsConnectionFrom(const Parameter *source) const override;

    QWidget *createWidget(NodeEditor *) const override;
    void updateWidget(QWidget *) const override;
    QVariant updateValue(QWidget *) const override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;
};

} // namespace keira

#endif // BOOLEANPARAMETER_H
