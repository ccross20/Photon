#ifndef KEIRA_ANYPARAMETER_H
#define KEIRA_ANYPARAMETER_H
#include "parameter.h"

namespace keira {

// A parameter whose type is determined at connection time.
// Accepts any type when unresolved. Once the first connection is made, the
// type locks permanently to the connected parameter's typeId, and subsequent
// connections must match that type.
class KEIRA_EXPORT AnyParameter : public Parameter
{
public:
    const static QByteArray ParameterId;

    AnyParameter();
    AnyParameter(const QByteArray &t_id, const QString &t_name,
                 int connectionOptions = AllowMultipleOutput | AllowSingleInput);
    ~AnyParameter();

    bool isResolved() const;
    void resolve(const QByteArray &resolvedTypeId);

    QByteArray typeId() const override;
    bool isGeneric() const override;
    bool acceptsConnectionFrom(const Parameter *source) const override;

    QWidget *createWidget(NodeEditor *) const override;
    void updateWidget(QWidget *) const override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace keira

#endif // KEIRA_ANYPARAMETER_H
