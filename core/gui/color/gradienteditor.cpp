#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QResizeEvent>
#include <QPushButton>
#include <QSettings>
#include <QInputDialog>
#include <QMenu>
#include "gui/decospinbox.h"
#include "gradienteditor.h"
#include "vektor/vektorartist.h"
#include "colorselectorwidget.h"
#include "exocore.h"
#include "project/project.h"
#include "library/library.h"
#include "library/gradientitem.h"
#include "gui/menu/optionmenu.h"

namespace exo {


GradientEditor::GradientEditor(const Gradient &gradient, EditorTypes types, QWidget *parent) : QWidget(parent),m_gradient(gradient)
{
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setContentsMargins(0,0,0,0);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setSpacing(12);
    hLayout->setContentsMargins(0,0,0,0);

    m_typeGroup = new ButtonGroup();
    m_typeGroup->setIsTextVisible(false);
    if(types != EditorTypes::NoTypes)
    {
        m_typeGroup->addIcon(core::createIcon("deco.core.gradient.type.linear"),"Linear");
        m_typeGroup->addIcon(core::createIcon("deco.core.gradient.type.radial"),"Radial");
        m_typeGroup->addIcon(core::createIcon("deco.core.gradient.type.conic"),"Conic");
        m_typeGroup->addIcon(core::createIcon("deco.core.gradient.type.diamond"),"Diamond");
        m_typeGroup->addIcon(core::createIcon("deco.core.gradient.type.square_root"),"Square Root");
    }


    if(types == EditorTypes::AllTypes)
    {
        m_typeGroup->addIcon(core::createIcon("deco.core.gradient.type.along"),"Along Stroke");
        m_typeGroup->addIcon(core::createIcon("deco.core.gradient.type.across"),"Across Stroke");
    }

    if(m_typeGroup->buttonCount() > 0)
    {
        m_typeGroup->setCurrentIndex(m_gradient.type);
        connect(m_typeGroup,SIGNAL(indexSelected(int)),this,SLOT(gradientTypeDropdownSlot(int)));
        hLayout->addWidget(m_typeGroup);
    }


/*
    m_overshootGroup = new ButtonGroup();
    m_overshootGroup->setIsTextVisible(false);
    m_overshootGroup->addIcon(createIcon("deco.core.gradient.overshoot.clamp"),"Clamp");
    m_overshootGroup->addIcon(createIcon("deco.core.gradient.overshoot.reflect"),"Reflect");
    m_overshootGroup->addIcon(createIcon("deco.core.gradient.overshoot.repeat"),"Repeat");

    m_overshootGroup->setCurrentIndex(m_gradient.overshoot);
    connect(m_overshootGroup,SIGNAL(indexSelected(int)),this,SLOT(gradientOvershootDropdownSlot(int)));
    hLayout->addWidget(m_overshootGroup);


    m_curveGroup = new ButtonGroup();
    m_curveGroup->setIsTextVisible(false);
    m_curveGroup->addIcon(createIcon("deco.core.gradient.curve.linear"),"Linear");
    m_curveGroup->addIcon(createIcon("deco.core.gradient.curve.bezier"),"Bezier");
    m_curveGroup->setCurrentIndex(m_gradient.mode);
    connect(m_curveGroup,SIGNAL(indexSelected(int)),this,SLOT(gradientBlendModeDropdownSlot(int)));
    hLayout->addWidget(m_curveGroup);

    m_ditherButton = new QPushButton(createIcon("deco.core.gradient.curve.linear"),"");
    m_ditherButton->setToolTip("Dither");
    m_ditherButton->setCheckable(true);
    m_ditherButton->setChecked(m_gradient.dither);
    connect(m_ditherButton,SIGNAL(toggled(bool)),this,SLOT(gradientDitherToggled(bool)));
    hLayout->addWidget(m_ditherButton);

*/
    QLabel *rotateLabel = new QLabel("");
    rotateLabel->setToolTip("Rotate");
    rotateLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    rotateLabel->setPixmap(core::createPixmapIcon("deco.core.rotate", QSize{15,20}));


    m_rotateSpin = new InteractiveDoubleSpinBox;
    m_rotateSpin->setSuffix("°");
    m_rotateSpin->setValue(m_gradient.angle);
    m_rotateSpin->setMaximumWidth(50);
    m_rotateSpin->setMaximumHeight(30);
    connect(m_rotateSpin,SIGNAL(valueChanged(double)),this,SLOT(rotateUpdated(double)));

    if(types != EditorTypes::NoTypes)
    {
        hLayout->addSpacing(8);
        hLayout->addWidget(rotateLabel);
        hLayout->addWidget(m_rotateSpin);
        hLayout->addSpacing(5);
    }


    QPushButton *flipButton = new QPushButton(core::createIcon("deco.core.gradient.flip"),"");
    flipButton->setToolTip("Flip Gradient");
    connect(flipButton,SIGNAL(clicked()),this,SLOT(flipGradient()));
    hLayout->addWidget(flipButton);

    hLayout->addStretch();

    m_menuButton = new QPushButton(QIcon{":/resources/icons/gear.png"},"");
    m_menuButton->setFlat(true);
    m_menuButton->setProperty("styleClass", "noFrame");
    hLayout->addWidget(m_menuButton);
    connect(m_menuButton,SIGNAL(clicked()),this,SLOT(openMenu()));

    vLayout->addLayout(hLayout);

    m_gradientBar = new GradientBar(gradient);
    connect(m_gradientBar,SIGNAL(gradientChanged(Gradient)),this,SLOT(gradientChangedSlot(Gradient)));
    connect(m_gradientBar,SIGNAL(stopSelected(unsigned)),this,SLOT(gradientStopSelected(unsigned)));
    connect(m_gradientBar,SIGNAL(stopDeselected()),this,SLOT(gradientStopDeselected()));
    connect(m_gradientBar,SIGNAL(beginEditing()),this,SLOT(beganEditingGradient()));
    connect(m_gradientBar,SIGNAL(endEditing()),this,SLOT(endedEditingGradient()));
    vLayout->addWidget(m_gradientBar);

    QHBoxLayout *insetLayout = new QHBoxLayout();
    insetLayout->setContentsMargins(30,0,30,0);
    m_colorWidget = new ColorSelectorWidget();
    m_colorWidget->setVisible(false);
    connect(m_colorWidget,SIGNAL(selectionChanged(Color)),this,SLOT(gradientColorChangedSlot(Color)));
    connect(m_colorWidget,SIGNAL(menuOpened()),this,SIGNAL(menuOpened()));
    connect(m_colorWidget,SIGNAL(menuClosed()),this,SIGNAL(menuClosed()));
    connect(m_colorWidget,SIGNAL(resized()),this,SIGNAL(resized()));
    insetLayout->addWidget(m_colorWidget);
    vLayout->addLayout(insetLayout);


    QSettings settings;
    settings.beginGroup("GradientEditor");

    if(exoApp->project())
    {
        if(settings.contains("gradientFolderId"))
        {
            LibraryItemPtr item = exoApp->project()->library()->findItemById(settings.value("gradientFolderId").toUuid());

            if(item)
                m_palette = new MiniPalette(static_cast<FolderItem*>(item), GradientItem::TypeId, this);
        }

        if(!m_palette)
        {
            LibraryItemPtr firstItem = exoApp->project()->library()->findFirstItemByType(GradientItem::TypeId);
            FolderItemPtr folder = exoApp->project()->library()->rootFolder();
            if(firstItem)
                folder = firstItem->parent();
            m_palette = new MiniPalette(folder, GradientItem::TypeId, this);
        }
    }
    else
        m_palette = new MiniPalette(exoApp->project()->library()->rootFolder(), GradientItem::TypeId, this);
    m_palette->setVisible(settings.value("palette", true).toBool());

    vLayout->addWidget(m_palette);
    connect(m_palette,SIGNAL(selectItem(LibraryItemPtr)),this,SLOT(selectPaletteItem(LibraryItemPtr)));
    connect(m_palette,SIGNAL(addItem(FolderItemPtr)),this,SLOT(paletteAddItem(FolderItemPtr)));
    connect(m_palette,SIGNAL(updateSwatch(LibraryItemPtr)),this,SLOT(paletteUpdateSwatch(LibraryItemPtr)));
    connect(m_palette,SIGNAL(resized()),this,SIGNAL(resized()));
    connect(m_palette,SIGNAL(menuOpened()),this,SIGNAL(menuOpened()));
    connect(m_palette,SIGNAL(menuClosed()),this,SIGNAL(menuClosed()));
    connect(m_palette,SIGNAL(folderChanged()),this,SLOT(saveState()));

    setLayout(vLayout);
    settings.endGroup();

    typeUpdated();
}

void GradientEditor::beganEditingGradient()
{
    emit beginEditing();
    m_isEditing = true;
}

void GradientEditor::endedEditingGradient()
{
    m_isEditing = false;
    emit endEditing();
}

void GradientEditor::typeUpdated()
{
    if(m_gradient.type == Gradient::TypeAlongStroke || m_gradient.type == Gradient::TypeAcrossStroke)
    {
        m_rotateSpin->setEnabled(false);
    }
    else
    {
        m_rotateSpin->setEnabled(true);
    }
}

void GradientEditor::flipGradient()
{
    m_gradient.flip();
    setGradient(m_gradient);
    emit gradientChanged(m_gradient);
    update();
}

void GradientEditor::setGradient(const Gradient &gradient)
{
    if(m_isEditing)
        return;
    m_gradient = gradient;
    m_typeGroup->blockSignals(true);
    m_gradientBar->blockSignals(true);
    m_rotateSpin->blockSignals(true);

    m_typeGroup->setCurrentIndex(m_gradient.type);
    m_gradientBar->setGradient(m_gradient);
    m_rotateSpin->setValue(m_gradient.angle);

    m_gradientBar->blockSignals(false);
    m_typeGroup->blockSignals(false);
    m_rotateSpin->blockSignals(false);

    update();
}

void GradientEditor::setIsTypeVisible(bool value)
{
    m_typeGroup->setVisible(value);
    update();
}

bool GradientEditor::isTypeVisible() const
{
    return m_typeGroup->isVisible();
}


void GradientEditor::openMenu()
{
    QMenu menu;

    OptionMenu *overshootMenu = new OptionMenu("Overshoot");
    overshootMenu->addOption(core::createIcon("deco.core.gradient.overshoot.clamp"),"Clamp");
    overshootMenu->addOption(core::createIcon("deco.core.gradient.overshoot.reflect"),"Reflect");
    overshootMenu->addOption(core::createIcon("deco.core.gradient.overshoot.repeat"),"Repeat");
    overshootMenu->setIndex(m_gradient.overshoot);
    connect(overshootMenu,SIGNAL(optionSelected(int)),this,SLOT(gradientOvershootDropdownSlot(int)));
    menu.addMenu(overshootMenu);

    OptionMenu *interpMenu = new OptionMenu("Interpolation");
    interpMenu->addOption(core::createIcon("deco.core.gradient.curve.linear"),"Linear");
    interpMenu->addOption(core::createIcon("deco.core.gradient.curve.bezier"),"Bezier");
    interpMenu->setIndex(m_gradient.mode);
    connect(interpMenu,SIGNAL(optionSelected(int)),this,SLOT(gradientBlendModeDropdownSlot(int)));
    menu.addMenu(interpMenu);

    OptionMenu *stepMenu = new OptionMenu("Steps");
    stepMenu->addOption("1024");
    stepMenu->addOption("512");
    stepMenu->addOption("64");
    stepMenu->addOption("48");
    stepMenu->addOption("24");
    stepMenu->addOption("16");
    stepMenu->addOption("12");
    stepMenu->addOption("8");
    stepMenu->addOption("6");
    stepMenu->addOption("4");

    int stepIndex = 0;
    switch(m_gradient.steps)
    {
        case 1024:
            stepIndex = 0;
            break;
        case 512:
            stepIndex = 1;
            break;
        case 64:
            stepIndex = 2;
            break;
        case 48:
            stepIndex = 3;
            break;
        case 24:
            stepIndex = 4;
            break;
        case 16:
            stepIndex = 5;
            break;
        case 12:
            stepIndex = 6;
            break;
        case 8:
            stepIndex = 7;
            break;
        case 6:
            stepIndex = 8;
            break;
        case 4:
            stepIndex = 9;
            break;
    }


    stepMenu->setIndex(stepIndex);
    connect(stepMenu,SIGNAL(optionSelected(int)),this,SLOT(gradientStepDropdownSlot(int)));
    menu.addMenu(stepMenu);

    QAction *ditherAction = menu.addAction("Dither");
    ditherAction->setCheckable(true);
    ditherAction->setChecked(m_gradient.dither);
    connect(ditherAction, SIGNAL(toggled(bool)), this, SLOT(gradientDitherToggled(bool)));

    menu.addAction(core::createIcon("deco.core.gradient.flip"),"Flip",[this](){flipGradient();});

    menu.addSeparator();
    QAction *paletteVisibleAction = menu.addAction("Show Palette");
    paletteVisibleAction->setCheckable(true);
    paletteVisibleAction->setChecked(m_palette->isVisible());
    connect(paletteVisibleAction,SIGNAL(triggered()),this,SLOT(togglePaletteVisibility()));

    QMenu *palettesMenu = menu.addMenu("Palettes");

    auto paletteItems = exoApp->project()->library()->findItemsByType(FolderItem::TypeId);
    for(auto item : paletteItems)
    {
        if(!static_cast<FolderItemPtr>(item)->containsType(GradientItem::TypeId))
            continue;
        QAction *paletteAction = palettesMenu->addAction(item->name());
        paletteAction->setCheckable(true);
        paletteAction->setChecked(static_cast<FolderItem*>(item) == m_palette->folder());
        paletteAction->setData(item->uniqueId());
    }

    palettesMenu->addSeparator();
    QAction *newPaletteAction = palettesMenu->addAction("Create New Folder");
    connect(newPaletteAction,SIGNAL(triggered()),this,SLOT(createPaletteTriggered()));

    connect(palettesMenu,SIGNAL(triggered(QAction*)),this,SLOT(paletteChangeTriggered(QAction*)));

    emit menuOpened();
    menu.exec(m_menuButton->mapToGlobal(QPoint(m_menuButton->width(),0)));
    emit menuClosed();

}


void GradientEditor::saveState()
{
    QSettings settings;
    settings.beginGroup("GradientEditor");
    settings.setValue("palette", m_palette->isVisible());
    settings.setValue("gradientFolderId", m_palette->folder()->uniqueId());
    settings.endGroup();
    settings.sync();
}

void GradientEditor::togglePaletteVisibility()
{
    m_palette->setVisible(!m_palette->isVisible());
    saveState();
    //update();
    emit resized();
}

void GradientEditor::createPaletteTriggered()
{    
    core::promptText("New Palette", "Palette",[this](QString t_value){
        FolderItem *item = new FolderItem(t_value);
        exoApp->project()->library()->addItem(item);
        m_palette->setFolder(item);
        saveState();
    });
}

void GradientEditor::paletteChangeTriggered(QAction *action)
{
    if(action->data().isNull())
        return;

    LibraryItemPtr item = exoApp->project()->library()->findItemById(action->data().toUuid());

    m_palette->setFolder(static_cast<FolderItem*>(item));
    saveState();
}

void GradientEditor::gradientStepDropdownSlot(int index)
{
    int steps = 0;
    switch(index)
    {
        case 0:
            steps = 1024;
            break;
        case 1:
            steps = 512;
            break;
        case 2:
            steps = 64;
            break;
        case 3:
            steps = 48;
            break;
        case 4:
            steps = 24;
            break;
        case 5:
            steps = 16;
            break;
        case 6:
            steps = 12;
            break;
        case 7:
            steps = 8;
            break;
        case 8:
            steps = 6;
            break;
        case 9:
            steps = 4;
            break;
    }

    emit beginEditing();
    m_gradient.steps = steps;
    m_gradientBar->setGradient(m_gradient);
    emit gradientChanged(m_gradient);
    emit endEditing();
}

void GradientEditor::gradientTypeDropdownSlot(int index)
{
    emit beginEditing();
    m_gradient.type = static_cast<Gradient::GradientType>(index);
    typeUpdated();
    emit gradientChanged(m_gradient);
    emit endEditing();
}

void GradientEditor::gradientOvershootDropdownSlot(int index)
{
    emit beginEditing();
    m_gradient.overshoot = static_cast<Gradient::OvershootMode>(index);
    emit gradientChanged(m_gradient);
    emit endEditing();
}

void GradientEditor::gradientDitherToggled(bool value)
{
    emit beginEditing();
    m_gradient.dither = value;
    emit gradientChanged(m_gradient);
    emit endEditing();
}

void GradientEditor::gradientBlendModeDropdownSlot(int index)
{
    emit beginEditing();
    m_gradient.mode = static_cast<Gradient::BlendMode>(index);
    emit gradientChanged(m_gradient);
    emit endEditing();
}

void GradientEditor::rotateUpdated(double value)
{
    emit beginEditing();
    m_gradient.angle = value;
    m_gradientBar->setGradient(m_gradient);
    emit gradientChanged(m_gradient);
    emit endEditing();
}

void GradientEditor::gradientChangedSlot(const Gradient &gradient)
{
    m_gradient.stops = gradient.stops;
    emit gradientChanged(m_gradient);
}

void GradientEditor::gradientStopSelected(unsigned stopIndex)
{
    m_selectedStop = stopIndex;
    m_colorWidget->setVisible(true);

    m_colorWidget->blockSignals(true);
    m_colorWidget->setColor(m_gradient.stops[stopIndex].color);
    m_colorWidget->blockSignals(false);
    emit resized();
}

void GradientEditor::gradientStopDeselected()
{
    m_colorWidget->setVisible(false);
    emit resized();
}

void GradientEditor::gradientColorChangedSlot(const Color &color)
{
    if(m_selectedStop >= m_gradient.size())
        return;
    emit beginEditing();
    m_gradient.stops[m_selectedStop].color = color;
    m_gradientBar->setGradient(m_gradient);
    emit gradientChanged(m_gradient);
    emit endEditing();
}

void GradientEditor::selectPaletteItem(LibraryItemPtr item)
{
    auto tempType = m_gradient.type;
    Gradient grad = static_cast<GradientItem*>(item)->gradient();
    grad.type = tempType;
    setGradient(grad);

    emit gradientChanged(m_gradient);
}

void GradientEditor::paletteAddItem(FolderItemPtr folder)
{
    folder->addChild(new GradientItem{m_gradient});
}

void GradientEditor::paletteUpdateSwatch(LibraryItemPtr item)
{

    static_cast<GradientItem*>(item)->setGradient(m_gradient);


}

} // namespace exo
