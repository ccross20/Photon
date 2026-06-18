#ifndef COLORPALETTEPARAMETER_H
#define COLORPALETTEPARAMETER_H
#include <QColor>
#include "photon-global.h"
#include "model/parameter/parameter.h"

namespace photon {

class PHOTONCORE_EXPORT ColorPaletteParameter : public keira::Parameter
{
public:
    const static QByteArray ParameterId;

    ColorPaletteParameter();
    ColorPaletteParameter(const QByteArray &t_id, const QString &t_name, ColorPalette t_default, int connectionOptions = keira::AllowMultipleOutput | keira::AllowSingleInput);
    ~ColorPaletteParameter();

    QWidget *createWidget(keira::NodeEditor *) const override;
    void updateWidget(QWidget *) const override;
    QVariant updateValue(QWidget *) const override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;

private:

};

} // namespace photon

#endif // COLORPALETTEPARAMETER_H
