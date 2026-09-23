#ifndef COLORSELECTORWIDGET_H
#define COLORSELECTORWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QVector>
#include <functional>
#include "colorslider.h"
#include "../photon-ui-global.h"
#include "huebox.h"
#include "simpleswatch.h"

namespace photon {

class PHOTONUI_EXPORT ColorSelectorWidget : public QWidget
{
    Q_OBJECT
public:
    enum Features
    {
        FeatureNone = 0,
        FeatureSlider = 0x1,
        FeaturePalette = 0x2,
        FeatureField = 0x4,
        FeatureMenu = 0x8,
        FeatureSliderEdit = 0x10,
        FeatureAll = FeatureSlider | FeaturePalette | FeatureField | FeatureMenu | FeatureSliderEdit
    };

    enum SliderMode{
        SliderModeRGBA,
        SliderModeHSVA,
        SliderModeHSLA,
        SliderModeNone
    };

    enum ColorFieldMode{
        FieldModeBox,
        FieldModeNone
    };

    explicit ColorSelectorWidget(const QColor &color = QColor(Qt::red), int features = FeatureAll, QWidget *parent = nullptr);
    ~ColorSelectorWidget();

    // photon-ui sits below core (see the project's layering rule) and can't
    // reach Project::colors() directly, so the app wires this up once at
    // startup (see PhotonCore::init()) instead of a direct dependency. Every
    // ColorSelectorWidget with FeaturePalette set reads through it to show a
    // clickable strip of the project's saved colours.
    using SavedColorsProvider = std::function<QVector<QColor>()>;
    static void setSavedColorsProvider(SavedColorsProvider provider);


signals:
    void selectionChanged(QColor color);
    void menuOpened();
    void menuClosed();
    void beginEditing();
    void endEditing();
    void resized();

public slots:
    void setSliderMode(ColorSelectorWidget::SliderMode mode);
    void setFieldMode(ColorSelectorWidget::ColorFieldMode mode);
    void setColor(const QColor &color);

private slots:

    void hueBoxChanged(const QColor &color);
    void sliderModeTriggered(QAction *action);
    void fieldModeTriggered(QAction *action);
    void sliderChanged(double value);
    void spinAChanged(double value);
    void spinBChanged(double value);
    void spinCChanged(double value);
    void spinDChanged(double value);
    void openMenu();
    void hexChanged(const QString &text);
    void hexReceivedFocus();
    void hexLostFocus();

    void saveState();

private:
    QDoubleSpinBox *createSpinBox(double value, double min = 0, double max = 1);
    void updateColor();
    void buildSliders();
    void buildSavedColorsRow(QVBoxLayout *parentLayout);
    void closeMenu();
    double processValue(double value);

    QGridLayout *m_sliderGrid;
    HueBox *m_hueBox;
    QLabel *m_sliderLabelA;
    QLabel *m_sliderLabelB;
    QLabel *m_sliderLabelC;
    QLabel *m_sliderLabelD;
    ColorSlider *m_sliderA;
    ColorSlider *m_sliderB;
    ColorSlider *m_sliderC;
    ColorSlider *m_sliderD;
    QDoubleSpinBox *m_spinA;
    QDoubleSpinBox *m_spinB;
    QDoubleSpinBox *m_spinC;
    QDoubleSpinBox *m_spinD;
    QLineEdit *m_hexEdit;
    SimpleSwatch *m_swatch;
    QPushButton *m_menuButton;
    QColor m_color;
    SliderMode m_sliderMode = SliderModeRGBA;
    ColorFieldMode m_fieldMode = FieldModeBox;
    int m_features;
    bool m_menuIsOpen = false;
    bool m_hexHasFocus = false;
    bool m_roundValues = true;
};

} // namespace photon

#endif // COLORSELECTORWIDGET_H
