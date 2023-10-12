#include <QPainter>
#include <QLabel>
#include <QPaintEvent>
#include <QTimer>
#include <QMenu>
#include <QInputDialog>
#include "minipalette.h"
#include "utils/qdrawingutils.h"
#include "exocore.h"
#include "project/project.h"
#include "library/library.h"
#include "settings/settings.h"
#include "help/helpmanager.h"

namespace exo {


PaletteSwatch::PaletteSwatch(LibraryItemPtr item, QSize size, QWidget *parent) : QWidget(parent),m_item(item),m_size(size)
{
    if(m_item)
    {
        m_item->addEventListener(this, "PaletteSwatch");
        setToolTip(m_item->name());
    }
}

PaletteSwatch::~PaletteSwatch()
{
    if(m_item)
        m_item->removeEventListener(this);
}

void PaletteSwatch::receiveEvent(int eventType, void *, void *, void *)
{
    switch (eventType) {
        case Event::LibraryItem_Updated:
            update();
        break;

        case Event::LibraryItem_WillDelete:
            m_item->removeEventListener(this);
            m_item = nullptr;
        break;
    }
}

void PaletteSwatch::setIsAddButton(bool value)
{
    m_isAddButton = value;
}

QSize PaletteSwatch::sizeHint() const
{
    return m_size;
}

void PaletteSwatch::paintEvent(QPaintEvent *)
{

    QPainter painter{this};

    if(m_isAddButton)
    {
        QColor fillColor = m_hover ? exoApp->settings()->accentColor() : Qt::white;

        int radius = 6;
        int thickness = 2;
        QPoint centerPt = rect().center();

        painter.fillRect(centerPt.x() - radius, centerPt.y() - thickness, radius * 2, thickness * 2, fillColor);
        painter.fillRect(centerPt.x() - thickness, centerPt.y() - radius, thickness * 2, radius * 2, fillColor);
    }
    else
        m_item->paint(&painter, QRect(0, 0, m_size.width(), m_size.height()));

    painter.setPen(QPen(m_hover ? exoApp->settings()->accentColor() : Qt::black, 2, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRect(1,1,width()-2, height()-2));
}

void PaletteSwatch::enterEvent(QEnterEvent *)
{
    m_hover = true;
    core::showHint(HelpHint{{HintOption{"Select Swatch", HintOption::Action_Click},
                             HintOption{"Delete Swatch", HintOption::Action_Alt},
                             HintOption{"Update Swatch", HintOption::Action_Control}}});
    update();
}

void PaletteSwatch::leaveEvent(QEvent *)
{
    core::clearHint();
    m_hover = false;
    update();
}

void PaletteSwatch::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::MiddleButton)
        return QWidget::mousePressEvent(event);
    if(m_isAddButton)
    {
        emit addItem();
        return;
    }

    if(event->modifiers() & Qt::ControlModifier)
    {
        emit updateItem(m_item);
    }
    else if(event->modifiers() & Qt::AltModifier)
    {
        m_item->removeEventListener(this);
        LibraryItemPtr item = m_item;
        QTimer::singleShot(50,[item](){
            item->deleteItem();
        });
        m_item = nullptr;
    }
    else
        emit selectItem(m_item);
}



PaletteGroup::PaletteGroup(PaletteGroupData *data, FolderItemPtr folder, QWidget *parent) :QWidget(parent), m_data(data),m_folder(folder)
{
    m_labelButton = new QPushButton(data->name);
    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setContentsMargins(0,0,0,0);
    vLayout->addWidget(m_labelButton);

    m_folder->addEventListener(this, "PaletteGroup");

    m_flowLayout = new FlowLayout(nullptr, 0,0,0);

    vLayout->addLayout(m_flowLayout);
    setLayout(vLayout);

    refresh();
}

PaletteGroup::~PaletteGroup()
{
    if(m_folder)
        m_folder->removeEventListener(this);
}

void PaletteGroup::refresh()
{
    QDrawingUtils::clearLayout(m_flowLayout);

    for(LibraryItemPtr item : m_data->items)
    {
        PaletteSwatch *swatch = new PaletteSwatch(item);
        m_flowLayout->addWidget(swatch);

        connect(swatch, SIGNAL(selectItem(LibraryItemPtr)), this, SIGNAL(selectItem(LibraryItemPtr)));
        connect(swatch, SIGNAL(updateItem(LibraryItemPtr)), this, SIGNAL(updateItem(LibraryItemPtr)));
        connect(swatch, SIGNAL(addItem()), this, SLOT(addItemClicked()));
    }

    PaletteSwatch *addSwatch = new PaletteSwatch(nullptr);
    addSwatch->setIsAddButton(true);
    m_flowLayout->addWidget(addSwatch);
    connect(addSwatch, SIGNAL(addItem()), this, SLOT(addItemClicked()));
}

void PaletteGroup::addItemClicked()
{
    emit addItem(m_folder);
}

void PaletteGroup::receiveEvent(int eventType, void *, void *, void *)
{
    switch (eventType) {

        case Event::LibraryItem_WillDelete:
            m_folder = nullptr;
        break;
    }
}



MiniPalette::MiniPalette(FolderItem *folderItem, LibraryTypeId type, QWidget *parent) : QWidget(parent),m_folderItem(folderItem),
  m_swatchSize(QSize(24,24)),m_type(type)
{

    m_vLayout = new QVBoxLayout;
    m_vLayout->setContentsMargins(0,0,0,0);
    setLayout(m_vLayout);

    if(m_folderItem)
    {
        m_folderItem->addEventListener(this,"Mini Palette");

    }

    QHBoxLayout *labelLayout = new QHBoxLayout;
    labelLayout->setContentsMargins(0,0,0,0);
    m_labelButton = new QPushButton("test");
    m_labelButton->setVisible(false);
    m_labelButton->setStyleSheet("border:0px;font-weight:bold;");
    m_labelButton->setIcon(QIcon(":/resources/icons/dropdown-arrows.png"));
    m_labelButton->setLayoutDirection(Qt::LayoutDirection::RightToLeft);
    labelLayout->addWidget(m_labelButton);
    labelLayout->addStretch();
    m_vLayout->addLayout(labelLayout);

    m_flowLayout = new FlowLayout(nullptr, 0,0,0);

    m_vLayout->addLayout(m_flowLayout);


    connect(m_labelButton, SIGNAL(clicked()), this, SLOT(titleClicked()));
    setMinimumSize(100,24);
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

    setFolder(folderItem);
}

MiniPalette::~MiniPalette()
{
    if(m_folderItem)
        m_folderItem->removeEventListener(this);


}

/*
void MiniPalette::recursiveAddFolder(FolderItemPtr folder, const QString &titlePath, bool addEvenIfEmpty)
{
    PaletteGroupData *groupData = new PaletteGroupData;
    groupData->name = titlePath + folder->name();

    for(auto item : folder->children())
    {
        if(item->isFolder())
            recursiveAddFolder(static_cast<FolderItemPtr>(item), groupData->name + " / ");

        if(m_types.contains(item->type()))
            groupData->items.append(item);
    }

    if(groupData->items.length() > 0 || addEvenIfEmpty)
    {
        m_groups.append(groupData);

        PaletteGroup *group = new PaletteGroup(groupData, folder);
        m_vLayout->addWidget(group);



        connect(group, SIGNAL(selectItem(LibraryItemPtr)), this, SIGNAL(selectItem(LibraryItemPtr)));
        connect(group, SIGNAL(updateItem(LibraryItemPtr)), this, SIGNAL(updateSwatch(LibraryItemPtr)));
        connect(group, SIGNAL(addItem(FolderItemPtr)), this, SIGNAL(addItem(FolderItemPtr)));


    }
    else
        delete groupData;
}
*/

void MiniPalette::setFolder(FolderItem *folder)
{
    if(m_folderItem)
    {
        m_folderItem->removeEventListener(this);
    }

    m_folderItem = folder;
    if(m_folderItem)
    {
        m_labelButton->setVisible(true);
        m_labelButton->setText(m_folderItem->name());
        m_folderItem->addEventListener(this,"Mini Palette");
    }
    else
        m_labelButton->setVisible(false);
    rebuildSwatches();
}

FolderItem *MiniPalette::folder() const
{
    return m_folderItem;
}

void MiniPalette::addItemClicked()
{
    emit addItem(m_folderItem);
}

void MiniPalette::titleClicked()
{
    QMenu menu(this);


    auto paletteItems = exoApp->project()->library()->findItemsByType(FolderItem::TypeId);
    for(auto item : paletteItems)
    {
        if(!static_cast<FolderItemPtr>(item)->containsType(m_type))
            continue;
        QAction *paletteAction = menu.addAction(item->name());
        paletteAction->setCheckable(true);
        paletteAction->setChecked(static_cast<FolderItem*>(item) == m_folderItem);
        paletteAction->setData(item->uniqueId());
    }

    menu.addSeparator();
    menu.addAction("Create New Palette");


    emit menuOpened();
    QAction *action = menu.exec(m_labelButton->mapToGlobal(QPoint(0, m_labelButton->height())));

    if(action)
    {
        if(action->data().isNull())
        {
            core::promptText("New Palette", "Palette",[this](QString t_value){
                FolderItem *item = new FolderItem(t_value);
                exoApp->project()->library()->addItem(item);
                setFolder(item);
                emit folderChanged();
            });
        }
        else
        {
            LibraryItemPtr item = exoApp->project()->library()->findItemById(action->data().toUuid());
            setFolder(static_cast<FolderItem*>(item));
            emit folderChanged();
        }
    }

    emit menuClosed();
}

void MiniPalette::rebuildSwatches()
{

    QDrawingUtils::clearLayout(m_flowLayout);

    if(m_folderItem)
    {
        for(LibraryItemPtr item : m_folderItem->children())
        {
            if(m_type != item->type())
                continue;

            PaletteSwatch *swatch = new PaletteSwatch(item);
            m_flowLayout->addWidget(swatch);

            connect(swatch, SIGNAL(selectItem(LibraryItemPtr)), this, SIGNAL(selectItem(LibraryItemPtr)));
            connect(swatch, SIGNAL(updateItem(LibraryItemPtr)), this, SIGNAL(updateSwatch(LibraryItemPtr)));
            connect(swatch, SIGNAL(addItem()), this, SLOT(addItemClicked()));
        }

        PaletteSwatch *addSwatch = new PaletteSwatch(nullptr);
        addSwatch->setIsAddButton(true);
        m_flowLayout->addWidget(addSwatch);
        connect(addSwatch, SIGNAL(addItem()), this, SLOT(addItemClicked()));
    }

    updateGeometry();
    update();
    emit resized();
}


void MiniPalette::receiveEvent(int eventType, void *source, void *data1, void *data2)
{
    Q_UNUSED(source)
    Q_UNUSED(data1)
    Q_UNUSED(data2)

    switch (eventType) {
        case Event::FolderItem_DidAdd:
        case Event::FolderItem_RelativeAdded:
        case Event::FolderItem_RelativeRemoved:
        case Event::FolderItem_DidRemove:
        case Event::FolderItem_DidMove:
            rebuildSwatches();
        break;
    }
}

} // namespace exo
