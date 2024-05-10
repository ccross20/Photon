#include <QDoubleSpinBox>
#include <QLabel>
#include <QHBoxLayout>
#include "point2dparameter.h"
#include "view/nodeitem.h"
#include "gui/pointedit.h"


namespace photon {

const QByteArray Point2DParameter::ParameterId = "point2D";

Point2DParameter::Point2DParameter()
{

}

Point2DParameter::Point2DParameter(const QByteArray &t_id, const QString &t_name, QPointF t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, t_default, connectionOptions)
{

}


QWidget *Point2DParameter::createWidget(keira::NodeItem *item) const
{

    //const Point2DParameter *param = this;

    PointEdit *pointEdit = new PointEdit;

    const Point2DParameter *param = this;
    PointEdit::connect(pointEdit, &PointEdit::valueChanged, pointEdit,[item, pointEdit, param](){item->widgetUpdated(pointEdit, param);});


    return pointEdit;
}

void Point2DParameter::updateWidget(QWidget *t_widget) const
{
    const QPointF pt = value().toPointF();

    PointEdit *pointEdit = static_cast<PointEdit*>(t_widget);
    pointEdit->setValue(pt);
}

QVariant Point2DParameter::updateValue(QWidget *t_widget) const
{
    PointEdit *pointEdit = dynamic_cast<PointEdit*>(t_widget);

    if(pointEdit)
        return pointEdit->value();
    else
        return QPoint();
}

void Point2DParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

}

void Point2DParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

}

} // namespace photon

