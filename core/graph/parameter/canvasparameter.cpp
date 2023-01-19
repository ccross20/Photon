#include <QImage>
#include "canvasparameter.h"

namespace photon {

const QByteArray CanvasParameter::ParameterId = "canvas";

class CanvasParameter::Impl
{
public:

};

CanvasParameter::CanvasParameter() : Parameter(),m_impl(new Impl)
{

}

CanvasParameter::CanvasParameter(const QByteArray &t_id, const QString &t_name, QImage t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, t_default, connectionOptions),m_impl(new Impl)
{

}

CanvasParameter::~CanvasParameter()
{
    delete m_impl;
}

void CanvasParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

}

void CanvasParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

}

} // namespace keira
