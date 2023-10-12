#ifndef GRADIENTEDITOR_H
#define GRADIENTEDITOR_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>

#include "exo-core_global.h"
#include "vektor/gradient.h"
#include "vektor/vektorrenderer.h"
#include "gradientbar.h"
#include "minipalette.h"
#include "gui/buttongroup.h"
#include "gui/cleandoublespinbox.h"
#include "gui/interactivedoublespinbox.h"
#include "gui/interactivespinbox.h"

namespace exo {

class ColorSelectorWidget;

class EXOCORE_EXPORT GradientEditor : public QWidget
{
    Q_OBJECT
public:

    enum EditorTypes
    {
        NoTypes,
        FillTypes,
        AllTypes
    };

    GradientEditor(const Gradient &gradient, EditorTypes types = NoTypes, QWidget *parent = nullptr);

    void setIsTypeVisible(bool value);
    bool isTypeVisible() const;

    void setGradient(const Gradient &gradient);
signals:
    void gradientChanged(Gradient gradient);
    void beginEditing();
    void endEditing();
    void menuOpened();
    void menuClosed();
    void resized();

public slots:
    void gradientTypeDropdownSlot(int index);
    void gradientOvershootDropdownSlot(int index);
    void gradientBlendModeDropdownSlot(int index);
    void gradientStepDropdownSlot(int index);
    void gradientDitherToggled(bool value);
    void gradientChangedSlot(const Gradient &gradient);
    void gradientStopSelected(unsigned stopIndex);
    void gradientStopDeselected();
    void gradientColorChangedSlot(const Color &color);
    void rotateUpdated(double value);

    void togglePaletteVisibility();
    void createPaletteTriggered();
    void paletteChangeTriggered(QAction *action);

    void selectPaletteItem(LibraryItemPtr item);
    void paletteAddItem(FolderItemPtr folder);
    void paletteUpdateSwatch(LibraryItemPtr item);


private slots:
    void beganEditingGradient();
    void endedEditingGradient();
    void flipGradient();
    void openMenu();
    void saveState();

private:
    void typeUpdated();

    MiniPalette *m_palette = nullptr;
    GradientBar *m_gradientBar;
    ColorSelectorWidget *m_colorWidget;
    Gradient m_gradient;
    ButtonGroup *m_typeGroup;
    QPushButton *m_menuButton;
    InteractiveDoubleSpinBox *m_rotateSpin;
    int m_gradientInset = 5;
    unsigned m_selectedStop;
    bool m_isTypeVisible = true;
    bool m_isEditing = false;
};

} // namespace exo

#endif // GRADIENTEDITOR_H
