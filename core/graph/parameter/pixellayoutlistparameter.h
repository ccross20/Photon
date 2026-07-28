#ifndef PHOTON_PIXELLAYOUTLISTPARAMETER_H
#define PHOTON_PIXELLAYOUTLISTPARAMETER_H

#include <QStringList>
#include "photon-global.h"
#include "model/parameter/parameter.h"

namespace photon {

// Selects zero or more of the project's pixel layouts (by uniqueId). Rendered in
// the node editor as a checkable list. Used by CanvasOutputNode to choose which
// fixtures/pixels that output drives.
class PHOTONCORE_EXPORT PixelLayoutListParameter : public keira::Parameter
{
public:
    const static QByteArray ParameterId;

    PixelLayoutListParameter();
    PixelLayoutListParameter(const QByteArray &t_id, const QString &t_name,
                             QStringList t_default = {}, int connectionOptions = keira::NoConnection);

    QWidget *createWidget(keira::NodeEditor *) const override;
    QVariant updateValue(QWidget *) const override;

    void readFromJson(const QJsonObject &) override;
    void writeToJson(QJsonObject &) const override;
};

} // namespace photon

#endif // PHOTON_PIXELLAYOUTLISTPARAMETER_H
