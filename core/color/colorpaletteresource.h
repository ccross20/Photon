#ifndef PHOTON_COLORPALETTERESOURCE_H
#define PHOTON_COLORPALETTERESOURCE_H

#include "photon-global.h"
#include "project/projectresource.h"

namespace photon {

// A single named, saved palette (an ordered list of colours) - shown in the
// project panel's "Color Palettes" section and reachable from the node graph
// via SavedPaletteNode.
class PHOTONCORE_EXPORT ColorPaletteResource : public QObject, public ProjectResource
{
    Q_OBJECT
public:
    ColorPaletteResource();
    ~ColorPaletteResource();

    QByteArray uniqueId() const;

    QString name() const;
    void setName(const QString &);

    ColorPalette palette() const;
    void setPalette(const ColorPalette &);

    // ProjectResource
    QByteArray resourceId() const override{return uniqueId();}
    QByteArray resourceTypeId() const override{return "color-palette";}
    QString resourceName() const override{return name();}
    void setResourceName(const QString &t_name) override{setName(t_name);}
    QObject *resourceObject() override{return this;}
    QWidget *createResourceEditor() override;

    void readFromJson(const QJsonObject &);
    void writeToJson(QJsonObject &) const;

private:
    class Impl;
    Impl *m_impl;
};

} // namespace photon

#endif // PHOTON_COLORPALETTERESOURCE_H
