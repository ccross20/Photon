#ifndef PALETTEGIZMO_H
#define PALETTEGIZMO_H
#include <QColor>
#include "surfacegizmo.h"

namespace photon {

class PHOTONCORE_EXPORT PaletteGizmo : public SurfaceGizmo
{
public:
    const static QByteArray GizmoId;

    PaletteGizmo();

    bool isSticky() const;
    void setIsSticky(bool);
    bool isPressed() const;
    void setIsPressed(bool);
    QColor selectedColor() const;
    bool hasSelectedColor() const;
    void selectColor(int);
    void setPalette(const ColorPalette &);

    const ColorPalette &palette() const{return m_palette;}


    SurfaceGizmoWidget *createWidget(SurfaceMode mode) override;

private:
    ColorPalette m_palette;
    int m_selectedIndex = -1;
    bool m_isSticky = false;
    bool m_isPressed = false;
};

} // namespace photon

#endif // PALETTEGIZMO_H
