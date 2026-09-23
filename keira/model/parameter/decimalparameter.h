#ifndef KEIRA_DECIMALPARAMETER_H
#define KEIRA_DECIMALPARAMETER_H
#include "parameter.h"

namespace keira {

class KEIRA_EXPORT DecimalParameter : public Parameter
{
public:

    const static QByteArray ParameterId;

    DecimalParameter();
    DecimalParameter(const QByteArray &t_id, const QString &t_name, double t_default, int connectionOptions =  AllowSingleInput);
    ~DecimalParameter();

    // Hard bounds: the absolute ceiling on the value, however it got there
    // (typed, scrubbed, or a connected wire). Defaults to unbounded.
    void setMinimum(double);
    void setMaximum(double);
    double minimum() const;
    double maximum() const;

    // Soft bounds: the interactive range the field's slider/fill and
    // scrub/wheel sensitivity are scaled to, independent of the hard range -
    // a value can still be typed (or wired in) past it, up to the hard bound.
    // Defaults to mirroring the hard bounds; call these to narrow it. See
    // NumberScrubField's class comment for the full rationale.
    void setSoftMinimum(double);
    void setSoftMaximum(double);
    void setSoftRange(double minimum, double maximum);
    double softMinimum() const;
    double softMaximum() const;
    bool hasSoftRange() const;

    void setPrecision(uint);

    void setValue(const QVariant &) override;

    // Also accept an integer source (widened losslessly to a double) and a
    // boolean one (false = 0, true = 1).
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

#endif // KEIRA_DECIMALPARAMETER_H
