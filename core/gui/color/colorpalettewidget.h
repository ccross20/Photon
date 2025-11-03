#ifndef COLORPALETTEWIDGET_H
#define COLORPALETTEWIDGET_H

#include <QWidget>
#include "photon-global.h"

namespace photon {

class PHOTONCORE_EXPORT ColorPaletteWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ColorPaletteWidget(const ColorPalette &palette, bool isEditable = false,QWidget *parent = nullptr);
    void setIsEditable(bool);
    bool isEditable() const;

    ColorPalette palette() const;

public slots:
    void setPalette(const photon::ColorPalette &palette);

private slots:
    void colorSelected(int index);
    void openAddColor();
    void openEditColor(int index);

signals:
    void paletteUpdated();
    void colorPicked(QColor);

private:
    void buildLayout();

    ColorPalette m_palette;
    bool m_isEditable;
};

} // namespace photon

#endif // COLORPALETTEWIDGET_H
