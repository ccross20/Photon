#include <QLabel>
#include <QJsonArray>
#include "matrixparameter.h"

namespace photon {

const QByteArray MatrixParameter::ParameterId = "matrix";

class MatrixParameter::Impl
{
public:

};

MatrixParameter::MatrixParameter() : Parameter(), m_impl(new Impl)
{

}

MatrixParameter::MatrixParameter(const QByteArray &t_id, const QString &t_name, QMatrix4x4 t_default, int connectionOptions) :
    Parameter(ParameterId, t_id, t_name, QVariant::fromValue(t_default), connectionOptions), m_impl(new Impl)
{

}

MatrixParameter::~MatrixParameter()
{
    delete m_impl;
}

namespace {
// A full 4x4 matrix has no legible single-line representation, and there's
// no sensible way to hand-edit one anyway - so the widget is always a
// read-only summary of just the translation, the part a user actually
// recognizes at a glance.
QString summaryFor(const QMatrix4x4 &t_matrix)
{
    const QVector3D pos = t_matrix.map(QVector3D());
    return QString("(%1, %2, %3)").arg(pos.x(), 0, 'f', 2).arg(pos.y(), 0, 'f', 2).arg(pos.z(), 0, 'f', 2);
}
}

QWidget *MatrixParameter::createWidget(keira::NodeEditor *) const
{
    QLabel *label = new QLabel();
    label->setText(summaryFor(value().value<QMatrix4x4>()));
    label->setMaximumHeight(30);
    label->setStyleSheet("background:transparent;");
    label->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
    return label;
}

void MatrixParameter::updateWidget(QWidget *t_widget) const
{
    static_cast<QLabel*>(t_widget)->setText(summaryFor(value().value<QMatrix4x4>()));
}

QVariant MatrixParameter::updateValue(QWidget *) const
{
    return value();   // always read-only - nothing in the widget to pull back out
}

void MatrixParameter::readFromJson(const QJsonObject &t_json)
{
    Parameter::readFromJson(t_json);

    const QJsonArray floats = t_json.value("value").toArray();
    QMatrix4x4 matrix;   // identity default if the array is missing/short
    if(floats.size() == 16)
    {
        // Row-major, matching the array both QMatrix4x4(const float*) expects
        // and the order writeToJson() below appends in - constData() is
        // column-major, which would silently transpose the matrix if mixed
        // with the row-major array constructor here.
        float data[16];
        for(int i = 0; i < 16; ++i)
            data[i] = float(floats.at(i).toDouble());
        matrix = QMatrix4x4(data);
    }
    setValue(QVariant::fromValue(matrix));
}

void MatrixParameter::writeToJson(QJsonObject &t_json) const
{
    Parameter::writeToJson(t_json);

    const QMatrix4x4 matrix = value().value<QMatrix4x4>();
    QJsonArray floats;
    for(int r = 0; r < 4; ++r)
        for(int c = 0; c < 4; ++c)
            floats.append(matrix(r, c));
    t_json.insert("value", floats);
}

} // namespace photon
