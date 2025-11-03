#include <QHBoxLayout>
#include <QPushButton>
#include "colorpalettewidget.h"
#include "util/utils.h"
#include "colorselectordialog.h"

namespace photon {


ColorPaletteWidget::ColorPaletteWidget(const ColorPalette &palette, bool isEditable,QWidget *parent): QWidget{parent},m_palette(palette),m_isEditable(isEditable)
{
    buildLayout();
}

void ColorPaletteWidget::setIsEditable(bool t_value)
{
    if(m_isEditable == t_value)
        return;

    m_isEditable = t_value;
    buildLayout();
}

bool ColorPaletteWidget::isEditable() const
{
    return m_isEditable;
}

ColorPalette ColorPaletteWidget::palette() const
{
    return m_palette;
}

void ColorPaletteWidget::setPalette(const photon::ColorPalette &t_palette)
{
    m_palette = t_palette;
    emit paletteUpdated();
}

void ColorPaletteWidget::colorSelected(int index)
{
    if(m_isEditable)
    {
        openEditColor(index);
    }
    else
        emit colorPicked(m_palette[index]);
}

void ColorPaletteWidget::openAddColor()
{
    ColorSelectorDialog *colorsWidget = new ColorSelectorDialog(nullptr);
    colorsWidget->setAttribute(Qt::WA_DeleteOnClose);
    colorsWidget->show();
    colorsWidget->raise();
    colorsWidget->activateWindow();

    m_palette.append(Qt::red);

    connect(colorsWidget,&ColorSelectorDialog::selectionChanged,this,[this](QColor color){
        m_palette.last() = color;
        buildLayout();
        emit paletteUpdated();
    });
}

void ColorPaletteWidget::openEditColor(int index)
{
    ColorSelectorDialog *colorsWidget = new ColorSelectorDialog(nullptr);
    colorsWidget->setAttribute(Qt::WA_DeleteOnClose);
    colorsWidget->show();
    colorsWidget->raise();
    colorsWidget->activateWindow();


    connect(colorsWidget,&ColorSelectorDialog::selectionChanged,this,[this, index](QColor color){
        m_palette[index] = color;
        buildLayout();
        emit paletteUpdated();
    });
}

void ColorPaletteWidget::buildLayout()
{
    QHBoxLayout *hLayout = nullptr;
    if(layout())
    {
        hLayout = static_cast<QHBoxLayout*>(layout());
        clearLayout(hLayout);
    }
    else
        hLayout = new QHBoxLayout;

    int counter = 0;
    for(const QColor color : m_palette)
    {
        QPushButton *colorButton = new QPushButton();
        colorButton->setProperty("index", counter++);
        colorButton->setStyleSheet("QPushButton { background-color: " + color.name() + ";}");

        hLayout->addWidget(colorButton);

        connect(colorButton, &QPushButton::clicked,this, [this, counter](){
            colorSelected(counter-1);
        });

    }


    if(m_isEditable)
    {

        QPushButton *emptyButton = new QPushButton("Add");
        emptyButton->setMaximumHeight(10000);
        connect(emptyButton, &QPushButton::clicked,this, [this](){
            openAddColor();
        });
        hLayout->addWidget(emptyButton);
    }


    setLayout(hLayout);
}

} // namespace photon
