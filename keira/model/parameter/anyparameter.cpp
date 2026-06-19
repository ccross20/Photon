#include <QLabel>
#include "anyparameter.h"

namespace keira {

const QByteArray AnyParameter::ParameterId = "any";

class AnyParameter::Impl
{
public:
    QByteArray resolvedTypeId;
    bool isResolved = false;
};

AnyParameter::AnyParameter()
    : Parameter(ParameterId, {}, {}, {}), m_impl(new Impl)
{
}

AnyParameter::AnyParameter(const QByteArray &t_id, const QString &t_name, int connectionOptions)
    : Parameter(ParameterId, t_id, t_name, {}, connectionOptions)
    , m_impl(new Impl)
{
}

AnyParameter::~AnyParameter()
{
    delete m_impl;
}

bool AnyParameter::isResolved() const
{
    return m_impl->isResolved;
}

void AnyParameter::resolve(const QByteArray &resolvedTypeId)
{
    if (m_impl->isResolved)
        return;
    m_impl->isResolved = true;
    m_impl->resolvedTypeId = resolvedTypeId;
}

QByteArray AnyParameter::typeId() const
{
    return m_impl->isResolved ? m_impl->resolvedTypeId : ParameterId;
}

bool AnyParameter::isGeneric() const
{
    return !m_impl->isResolved;
}

bool AnyParameter::acceptsConnectionFrom(const Parameter *source) const
{
    // Unresolved: accept anything. Resolved: only accept the locked type or
    // another unresolved generic (which will itself lock to match).
    return !m_impl->isResolved
        || source->typeId() == m_impl->resolvedTypeId
        || source->isGeneric();
}

QWidget *AnyParameter::createWidget(NodeEditor *) const
{
    auto *label = new QLabel(m_impl->isResolved ? QString(m_impl->resolvedTypeId) : QStringLiteral("Any"));
    label->setMaximumHeight(24);
    return label;
}

void AnyParameter::updateWidget(QWidget *widget) const
{
    if (auto *label = qobject_cast<QLabel *>(widget))
        label->setText(m_impl->isResolved ? QString(m_impl->resolvedTypeId) : QStringLiteral("Any"));
}

void AnyParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);
    const QString resolved = t_json.value("resolvedTypeId").toString();
    if (!resolved.isEmpty())
        resolve(resolved.toLatin1());
}

void AnyParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);
    // Keep typeId as "any" in the file so deserialization recreates AnyParameter,
    // not the resolved concrete type.
    t_json.insert("typeId", QString(ParameterId));
    if (m_impl->isResolved)
        t_json.insert("resolvedTypeId", QString(m_impl->resolvedTypeId));
}

} // namespace keira
