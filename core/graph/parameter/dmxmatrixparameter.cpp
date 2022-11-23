#include "dmxmatrixparameter.h"

namespace photon {

const QByteArray DMXMatrixParameter::ParameterId = "dmxMatrix";

class DMXMatrixParameter::Impl
{
public:

};

DMXMatrixParameter::DMXMatrixParameter(): Parameter(),m_impl(new Impl)
{

}

DMXMatrixParameter::DMXMatrixParameter(const QByteArray &t_id, const QString &t_name, DMXMatrix t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, t_default, connectionOptions),m_impl(new Impl)
{

}

DMXMatrixParameter::~DMXMatrixParameter()
{
    delete m_impl;
}

void DMXMatrixParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);
}

void DMXMatrixParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);
}

} // namespace photon
