#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QActionGroup>
#include <QInputDialog>
#include <QSettings>
#include <QMenu>
#include "colorselectorwidget.h"

namespace photon {

ColorSelectorWidget::ColorSelectorWidget(const QColor &color, int features, QWidget *parent) : QWidget(parent),m_color(color.convertTo(QColor::Hsv)),m_features(features)
{

    QSettings settings;
    settings.beginGroup("ColorSelector");
    m_sliderMode = static_cast<SliderMode>(settings.value("slider", SliderModeRGBA).toInt());
    m_fieldMode = static_cast<ColorFieldMode>(settings.value("field", FieldModeBox).toInt());

    m_hueBox = new HueBox(this);
    m_hueBox->setMinimumHeight(80);
    connect(m_hueBox,SIGNAL(colorChange(QColor)),this,SLOT(hueBoxChanged(QColor)));
    connect(m_hueBox,SIGNAL(beginEditing()),this,SIGNAL(beginEditing()));
    connect(m_hueBox,SIGNAL(endEditing()),this,SIGNAL(endEditing()));
    m_hueBox->setVisible(m_fieldMode == FieldModeBox);

    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0,0,0,0);
    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0,0,0,0);


    QHBoxLayout *swatchLayout = new QHBoxLayout();
    swatchLayout->setContentsMargins(0,0,0,0);


    m_swatch = new SimpleSwatch();
    m_swatch->setMinimumHeight(50);
    //m_swatch->setMinimumWidth(20);
    //m_swatch->setMaximumWidth(40);
    swatchLayout->addWidget(m_swatch);

    QVBoxLayout *menuButtonLayout = new QVBoxLayout;
    menuButtonLayout->setContentsMargins(0,0,0,0);

    m_menuButton = new QPushButton(QIcon{":/resources/icons/gear.png"},"");
    m_menuButton->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    m_menuButton->setFlat(true);
    m_menuButton->setProperty("styleClass", "noFrame");



    if(m_features & FeatureMenu)
        menuButtonLayout->addWidget(m_menuButton);
    swatchLayout->addLayout(menuButtonLayout);

    vLayout->addLayout(swatchLayout);


    if(m_features & FeatureField)
    {
        hLayout->addWidget(m_hueBox);
    }



    //setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum));

    QVBoxLayout *componentVLayout = new QVBoxLayout();
    componentVLayout->setContentsMargins(0,0,0,0);

    m_sliderGrid = new QGridLayout();

    componentVLayout->addLayout(m_sliderGrid);


    m_hexEdit = new QLineEdit(this);
    m_hexEdit->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    m_hexEdit->setText(m_color.name(QColor::HexRgb));
    connect(m_hexEdit,&QLineEdit::textEdited,this,&ColorSelectorWidget::hexChanged);

    buildSliders();

    if(m_features & FeatureSlider)
        hLayout->addLayout(componentVLayout);

    vLayout->addLayout(hLayout);

    componentVLayout->addWidget(m_hexEdit);

    connect(m_menuButton,SIGNAL(clicked()),this,SLOT(openMenu()));


    setLayout(vLayout);
    m_hueBox->setValue(m_color);
    m_swatch->setColor(m_color);

    settings.endGroup();

}

ColorSelectorWidget::~ColorSelectorWidget()
{
    //disconnect(m_hexEdit,SIGNAL(editingFinished()),this,SLOT(hexChanged()));
}

QDoubleSpinBox *ColorSelectorWidget::createSpinBox(double value, double min, double max)
{
    QDoubleSpinBox* result = new QDoubleSpinBox();
    switch (m_sliderMode) {
    case photon::ColorSelectorWidget::SliderModeRGBA:
        result->setRange(min,max);
        result->setDecimals(0);
        break;
    case photon::ColorSelectorWidget::SliderModeHSLA:
    case photon::ColorSelectorWidget::SliderModeHSVA:
        result->setRange(min,max);
        result->setDecimals(2);
        break;

    case photon::ColorSelectorWidget::SliderModeNone:
        break;
    }

    result->setProperty("styleClass","parameterField");
    result->setValue(value);
    result->setButtonSymbols(QAbstractSpinBox::ButtonSymbols::NoButtons);
    result->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));

    return result;
}

void ColorSelectorWidget::openMenu()
{
    QMenu menu;

    if(m_features & FeatureSlider)
    {
        QMenu *colorModeMenu = menu.addMenu("Sliders");
        connect(colorModeMenu,SIGNAL(triggered(QAction*)),this,SLOT(sliderModeTriggered(QAction*)));

        QAction *rgbaAction = colorModeMenu->addAction("RGBA");
        rgbaAction->setCheckable(true);
        rgbaAction->setChecked(m_sliderMode == SliderModeRGBA);
        rgbaAction->setData(SliderModeRGBA);

        QAction *hsvaAction = colorModeMenu->addAction("HSVA");
        hsvaAction->setCheckable(true);
        hsvaAction->setChecked(m_sliderMode == SliderModeHSVA);
        hsvaAction->setData(SliderModeHSVA);

        QAction *hslaAction = colorModeMenu->addAction("HSLA");
        hslaAction->setCheckable(true);
        hslaAction->setChecked(m_sliderMode == SliderModeHSLA);
        hslaAction->setData(SliderModeHSLA);

        QAction *sliderNoneAction = colorModeMenu->addAction("None");
        sliderNoneAction->setCheckable(true);
        sliderNoneAction->setChecked(m_sliderMode == SliderModeNone);
        sliderNoneAction->setData(SliderModeNone);
    }

    if(m_features & FeatureField)
    {
        QMenu *colorFieldMenu = menu.addMenu("Editor Mode");
        connect(colorFieldMenu,SIGNAL(triggered(QAction*)),this,SLOT(fieldModeTriggered(QAction*)));

        QAction *boxAction = colorFieldMenu->addAction("Box");
        boxAction->setCheckable(true);
        boxAction->setChecked(m_fieldMode == FieldModeBox);
        boxAction->setData(FieldModeBox);


        QAction *noneAction = colorFieldMenu->addAction("None");
        noneAction->setCheckable(true);
        noneAction->setChecked(m_fieldMode == FieldModeNone);
        noneAction->setData(FieldModeNone);
    }

    emit menuOpened();
    m_menuIsOpen = true;
    menu.exec(m_menuButton->mapToGlobal(QPoint(m_menuButton->width(),0)));
    closeMenu();

}

void ColorSelectorWidget::closeMenu()
{
    emit menuClosed();
    m_menuIsOpen = false;

}


void ColorSelectorWidget::sliderModeTriggered(QAction *action)
{
    setSliderMode(static_cast<SliderMode>(action->data().toInt()));
    saveState();
}

void ColorSelectorWidget::fieldModeTriggered(QAction *action)
{
    setFieldMode(static_cast<ColorFieldMode>(action->data().toInt()));
    saveState();
}

void ColorSelectorWidget::saveState()
{
    QSettings settings;
    settings.beginGroup("ColorSelector");
    settings.setValue("slider", m_sliderMode);
    settings.setValue("field", m_fieldMode);

    settings.endGroup();
    settings.sync();
    emit resized();
}


void ColorSelectorWidget::setSliderMode(SliderMode mode)
{
    if(m_sliderMode == mode)
        return;
    m_sliderMode = mode;

    buildSliders();
}

void ColorSelectorWidget::setFieldMode(ColorFieldMode mode)
{
    if(m_fieldMode == mode)
        return;
    m_fieldMode = mode;

    switch (m_fieldMode) {
        case FieldModeBox:
            m_hueBox->setVisible(true);
        break;


        case FieldModeNone:
            m_hueBox->setVisible(false);
        break;
    }
}

void ColorSelectorWidget::buildSliders()
{
    //QDrawingUtils::clearLayout(m_sliderGrid);

    if(m_sliderMode == SliderModeNone)
    {
        m_hexEdit->setVisible(false);
        m_hueBox->setShowAlpha(true);
        return;
    }
    m_hexEdit->setVisible(true);
    m_hueBox->setShowAlpha(false);

    if(m_sliderMode == SliderModeRGBA)
    {
        m_sliderLabelA = new QLabel("R");
        m_sliderLabelA->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
        m_sliderA = new ColorSlider(m_color, ColorSlider::Mode_RGBA_Red, m_color.redF(),this);

        m_sliderLabelB = new QLabel("G");
        m_sliderLabelB->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
        m_sliderB = new ColorSlider(m_color, ColorSlider::Mode_RGBA_Green, m_color.greenF(),this);

        m_sliderLabelC = new QLabel("B");
        m_sliderLabelC->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
        m_sliderC = new ColorSlider(m_color, ColorSlider::Mode_RGBA_Blue, m_color.blueF(),this);

        m_sliderLabelD = new QLabel("A");
        m_sliderLabelD->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
        m_sliderD = new ColorSlider(m_color, ColorSlider::Mode_RGBA_Alpha, m_color.alphaF(),this);


            m_spinA = createSpinBox(m_color.red(),0,255);
            m_spinB = createSpinBox(m_color.green(),0,255);
            m_spinC = createSpinBox(m_color.blue(),0,255);
            m_spinD = createSpinBox(m_color.alpha(),0,255);



    }
    else if(m_sliderMode == SliderModeHSVA)
    {
        float h,s,v, a;
        m_color.getHsvF(&h, &s, &v, &a);

        m_sliderLabelA = new QLabel("H");
        m_sliderA = new ColorSlider(m_color, ColorSlider::Mode_HSVA_Hue, h);
        m_spinA = createSpinBox(processValue(h),0,360);
        m_spinA->setSuffix("°");

        m_sliderLabelB = new QLabel("S");
        m_sliderB = new ColorSlider(m_color, ColorSlider::Mode_HSVA_Saturation, s);
        m_spinB = createSpinBox(processValue(s), 0, 255);
        m_spinB->setSuffix("%");

        m_sliderLabelC = new QLabel("V");
        m_sliderC = new ColorSlider(m_color, ColorSlider::Mode_HSVA_Value, v);
        m_spinC = createSpinBox(processValue(v), 0, 255);
        m_spinC->setSuffix("%");

        m_sliderLabelD = new QLabel("A");
        m_sliderD = new ColorSlider(m_color, ColorSlider::Mode_HSVA_Alpha, a);
        m_spinD = createSpinBox(processValue(a), 0 , 255);
        m_spinD->setSuffix("%");
    }
    else if(m_sliderMode == SliderModeHSLA)
    {

        float h,s,l, a;
        m_color.getHslF(&h, &s, &l, &a);

        m_sliderLabelA = new QLabel("H");
        m_sliderA = new ColorSlider(m_color, ColorSlider::Mode_HSLA_Hue, h);
        m_spinA = createSpinBox(processValue(h), 0, 360);
        m_spinA->setSuffix("°");

        m_sliderLabelB = new QLabel("S");
        m_sliderB = new ColorSlider(m_color, ColorSlider::Mode_HSLA_Saturation, s);
        m_spinB = createSpinBox(processValue(s), 0 , 255);
        m_spinB->setSuffix("%");

        m_sliderLabelC = new QLabel("L");
        m_sliderC = new ColorSlider(m_color, ColorSlider::Mode_HSLA_Lightness, l);
        m_spinC = createSpinBox(processValue(l), 0 , 255);
        m_spinC->setSuffix("%");

        m_sliderLabelD = new QLabel("A");
        m_sliderD = new ColorSlider(m_color, ColorSlider::Mode_HSLA_Alpha, a);
        m_spinD = createSpinBox(processValue(a), 0 , 255);
        m_spinD->setSuffix("%");
    }

    connect(m_sliderA,SIGNAL(valueChange(double)),this,SLOT(sliderChanged(double)));
    connect(m_sliderA,SIGNAL(beginEditing()),this,SIGNAL(beginEditing()));
    connect(m_sliderA,SIGNAL(endEditing()),this,SIGNAL(endEditing()));
    connect(m_spinA,SIGNAL(valueChanged(double)),this,SLOT(spinAChanged(double)));

    connect(m_sliderB,SIGNAL(valueChange(double)),this,SLOT(sliderChanged(double)));
    connect(m_sliderB,SIGNAL(beginEditing()),this,SIGNAL(beginEditing()));
    connect(m_sliderB,SIGNAL(endEditing()),this,SIGNAL(endEditing()));
    connect(m_spinB,SIGNAL(valueChanged(double)),this,SLOT(spinBChanged(double)));

    connect(m_sliderC,SIGNAL(valueChange(double)),this,SLOT(sliderChanged(double)));
    connect(m_sliderC,SIGNAL(beginEditing()),this,SIGNAL(beginEditing()));
    connect(m_sliderC,SIGNAL(endEditing()),this,SIGNAL(endEditing()));
    connect(m_spinC,SIGNAL(valueChanged(double)),this,SLOT(spinCChanged(double)));

    connect(m_sliderD,SIGNAL(valueChange(double)),this,SLOT(sliderChanged(double)));
    connect(m_sliderD,SIGNAL(beginEditing()),this,SIGNAL(beginEditing()));
    connect(m_sliderD,SIGNAL(endEditing()),this,SIGNAL(endEditing()));
    connect(m_spinD,SIGNAL(valueChanged(double)),this,SLOT(spinDChanged(double)));

    if(m_features & FeatureSlider)
    {
        m_sliderGrid->addWidget(m_sliderLabelA,0,0);
        m_sliderGrid->addWidget(m_sliderA,0,1);
        m_sliderGrid->addWidget(m_sliderLabelB,1,0);
        m_sliderGrid->addWidget(m_sliderB,1,1);
        m_sliderGrid->addWidget(m_sliderLabelC,2,0);
        m_sliderGrid->addWidget(m_sliderC,2,1);
        m_sliderGrid->addWidget(m_sliderLabelD,3,0);
        m_sliderGrid->addWidget(m_sliderD,3,1);
    }

    if(m_features & FeatureSliderEdit)
    {
        m_sliderGrid->addWidget(m_spinA,0,2);
        m_sliderGrid->addWidget(m_spinB,1,2);
        m_sliderGrid->addWidget(m_spinC,2,2);
        m_sliderGrid->addWidget(m_spinD,3,2);
    }

}

void ColorSelectorWidget::setColor(const QColor &color)
{
    m_color = color;

    switch (m_sliderMode) {
    case photon::ColorSelectorWidget::SliderModeRGBA:
        m_color = m_color.convertTo(QColor::Hsv);
        break;
    case photon::ColorSelectorWidget::SliderModeHSVA:
        m_color = m_color.convertTo(QColor::Hsv);
        break;
    case photon::ColorSelectorWidget::SliderModeHSLA:
        m_color = m_color.convertTo(QColor::Hsl);
        break;
    case photon::ColorSelectorWidget::SliderModeNone:
        break;

    }



    updateColor();
}

void ColorSelectorWidget::updateColor()
{
    if(m_sliderMode == SliderModeNone)
    {
        m_hueBox->setValue(m_color);
        m_swatch->setColor(m_color);
        emit selectionChanged(m_color);
        return;
    }
    m_sliderA->blockSignals(true);
    m_sliderB->blockSignals(true);
    m_sliderC->blockSignals(true);
    m_sliderD->blockSignals(true);
    m_spinA->blockSignals(true);
    m_spinB->blockSignals(true);
    m_spinC->blockSignals(true);
    m_spinD->blockSignals(true);
    m_hexEdit->blockSignals(true);

    switch (m_sliderMode) {
    default:
    case photon::ColorSelectorWidget::SliderModeRGBA:
        {
            m_color = m_color.convertTo(QColor::Rgb);

            m_spinA->setValue(m_color.red());
            m_spinB->setValue(m_color.green());
            m_spinC->setValue(m_color.blue());
            m_spinD->setValue(m_color.alpha());

        }
        break;
    case photon::ColorSelectorWidget::SliderModeHSVA:
        {
            m_color = m_color.convertTo(QColor::Hsv);

            m_spinA->setValue(processValue(m_color.hue()));
            m_spinB->setValue(processValue(m_color.saturation()));
            m_spinC->setValue(processValue(m_color.value()));
            m_spinD->setValue(processValue(m_color.alpha()));
        }
        break;
    case photon::ColorSelectorWidget::SliderModeHSLA:
        {
            m_color = m_color.convertTo(QColor::Hsl);

            m_spinA->setValue(processValue(m_color.hue()));
            m_spinB->setValue(processValue(m_color.saturation()));
            m_spinC->setValue(processValue(m_color.lightness()));
            m_spinD->setValue(processValue(m_color.alpha()));

        }
        break;

    }



    m_sliderA->setColor(m_color);
    m_sliderB->setColor(m_color);
    m_sliderC->setColor(m_color);
    m_sliderD->setColor(m_color);

    m_hueBox->setValue(m_color);
    m_swatch->setColor(m_color);

    if(!m_hexHasFocus)
        m_hexEdit->setText(m_color.name(QColor::HexRgb));

    m_hexEdit->blockSignals(false);
    m_sliderA->blockSignals(false);
    m_sliderB->blockSignals(false);
    m_sliderC->blockSignals(false);
    m_sliderD->blockSignals(false);
    m_spinA->blockSignals(false);
    m_spinB->blockSignals(false);
    m_spinC->blockSignals(false);
    m_spinD->blockSignals(false);

    emit selectionChanged(m_color);
}

double ColorSelectorWidget::processValue(double value)
{
    if(m_roundValues)
        return round(value);
    return value;
}

void ColorSelectorWidget::sliderChanged(double value)
{
    Q_UNUSED(value);
    if(m_sliderMode == SliderModeNone)
        return;

    switch (m_sliderMode) {
        default:
        case ColorSelectorWidget::SliderModeRGBA:
            m_color = m_color.convertTo(QColor::Rgb);
            m_color.setRedF(m_sliderA->value());
            m_color.setGreenF(m_sliderB->value());
            m_color.setBlueF(m_sliderC->value());
            m_color = m_color.convertTo(QColor::Hsv);
            break;
        case ColorSelectorWidget::SliderModeHSVA:
            m_color = QColor::fromHsvF(m_sliderA->value(), m_sliderB->value(), m_sliderC->value(), m_color.alphaF());
            break;
        case ColorSelectorWidget::SliderModeHSLA:
            m_color = QColor::fromHslF(m_sliderA->value(), m_sliderB->value(), m_sliderC->value(), m_color.alphaF());
            break;
    }
    m_color.setAlphaF(m_sliderD->value());

    updateColor();
}

void ColorSelectorWidget::spinAChanged(double value)
{
    emit beginEditing();
    switch (m_sliderMode) {
        default:
        case ColorSelectorWidget::SliderModeRGBA:

            m_color = m_color.convertTo(QColor::Rgb);
            m_color.setRed(value);

            m_color = m_color.convertTo(QColor::Rgb);
            break;
        case ColorSelectorWidget::SliderModeHSVA:
        {
            int h,s,v,a;
            m_color.getHsv(&h, &s, &v, &a);
            m_color.setHsv(value,s,v,a);
        }
            break;
        case ColorSelectorWidget::SliderModeHSLA:
        {
            int h,s,l,a;
            m_color.getHsl(&h, &s, &l, &a);
            m_color.setHsl(value,s,l,a);
        }
            break;
    }

    updateColor();
    emit endEditing();
}

void ColorSelectorWidget::spinBChanged(double value)
{
    emit beginEditing();
    switch (m_sliderMode) {
        default:
        case ColorSelectorWidget::SliderModeRGBA:

            m_color = m_color.convertTo(QColor::Rgb);
            m_color.setGreen(value);

            m_color = m_color.convertTo(QColor::Rgb);
            break;
        case ColorSelectorWidget::SliderModeHSVA:
        {
            int h,s,v,a;
            m_color.getHsv(&h, &s, &v, &a);
            m_color.setHsv(h,value,v,a);
        }
            break;
        case ColorSelectorWidget::SliderModeHSLA:
        {
            int h,s,l,a;
            m_color.getHsl(&h, &s, &l, &a);
            m_color.setHsl(h,value,l,a);
        }
            break;
    }
    updateColor();
    emit endEditing();
}

void ColorSelectorWidget::spinCChanged(double value)
{
    emit beginEditing();
    switch (m_sliderMode) {
        default:
        case ColorSelectorWidget::SliderModeRGBA:

            m_color = m_color.convertTo(QColor::Rgb);
            m_color.setBlue(value);

            m_color = m_color.convertTo(QColor::Rgb);
            break;
        case ColorSelectorWidget::SliderModeHSVA:
        {
            int h,s,v,a;
            m_color.getHsv(&h, &s, &v, &a);
            m_color.setHsv(h,s,value,a);
        }
            break;
        case ColorSelectorWidget::SliderModeHSLA:
        {
            int h,s,l,a;
            m_color.getHsl(&h, &s, &l, &a);
            m_color.setHsl(h,s,value,a);
        }
            break;
    }
    updateColor();
    emit endEditing();
}

void ColorSelectorWidget::spinDChanged(double value)
{
    emit beginEditing();
    m_color.setAlpha(value);

    updateColor();
    emit endEditing();
}

void ColorSelectorWidget::hexChanged(const QString &text)
{
    emit beginEditing();

    QString fixedText = text;

    if(text.startsWith("0x"))
        fixedText = fixedText.right(fixedText.length()-2);
    else if(text.startsWith("#"))
        fixedText = fixedText.right(fixedText.length()-1);

    if(fixedText.length() > 6)
        fixedText = fixedText.left(6);

    fixedText.prepend('#');

    m_color = QColor(fixedText.leftJustified(7,'0'));
    updateColor();
    emit endEditing();
}

void ColorSelectorWidget::hexReceivedFocus()
{
    m_hexHasFocus = true;
}

void ColorSelectorWidget::hexLostFocus()
{
    m_hexHasFocus = false;
}

void ColorSelectorWidget::hueBoxChanged(const QColor &color)
{
    m_color = color.convertTo(QColor::Hsv);
    updateColor();

}

} // namespace exo
