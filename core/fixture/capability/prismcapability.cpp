#include <QRegularExpression>
#include "prismcapability.h"

namespace photon {

class PrismCapability::Impl
{
public:
    int facets = 3;
    bool linear = false;
};

PrismCapability::PrismCapability() : FixtureCapability(DMXRange(), Capability_Prism), m_impl(new Impl)
{
}

PrismCapability::~PrismCapability()
{
    delete m_impl;
}

int PrismCapability::facetCount() const
{
    return m_impl->facets;
}

bool PrismCapability::isLinear() const
{
    return m_impl->linear;
}

void PrismCapability::readFromOpenFixtureJson(const QJsonObject &t_json)
{
    FixtureCapability::readFromOpenFixtureJson(t_json);

    // The facet count is only carried in the free-text comment, e.g. "8-facet prism"
    // or "4-facet linear prism".
    const QString comment = t_json.value("comment").toString();
    const QRegularExpression re("(\\d+)\\s*-?\\s*facet", QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch m = re.match(comment);
    if (m.hasMatch())
        m_impl->facets = qMax(2, m.captured(1).toInt());

    m_impl->linear = comment.contains("linear", Qt::CaseInsensitive);
}

} // namespace photon
