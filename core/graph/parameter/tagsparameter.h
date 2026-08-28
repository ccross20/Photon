#ifndef PHOTON_TAGSPARAMETER_H
#define PHOTON_TAGSPARAMETER_H

#include "photon-global.h"
#include "model/parameter/parameter.h"

namespace photon {

// A space-separated list of tags, edited with the same colored chip picker
// (TagEditorWidget) used everywhere else tags are edited - autocompleting
// from every tag currently in use in the project, rather than a plain text
// field. The stored value stays a plain space-separated QString, the same
// shape the keira::StringParameter this replaces already used, so existing
// saved graphs keep loading unchanged; only the widget changes.
class PHOTONCORE_EXPORT TagsParameter : public keira::Parameter
{
public:
    const static QByteArray ParameterId;

    TagsParameter();
    TagsParameter(const QByteArray &id, const QString &name, const QString &defaultValue,
                 int connectionOptions = keira::AllowSingleInput);

    QWidget *createWidget(keira::NodeEditor *) const override;
    void updateWidget(QWidget *) const override;
    QVariant updateValue(QWidget *) const override;
};

} // namespace photon

#endif // PHOTON_TAGSPARAMETER_H
