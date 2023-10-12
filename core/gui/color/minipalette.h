#ifndef MINIPALETTE_H
#define MINIPALETTE_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include "library/paletteitem.h"
#include "event/ieventlistener.h"
#include "library/coloritem.h"
#include "gui/flowlayout.h"

namespace exo {



class PaletteSwatch : public QWidget, protected IEventListener
{
    Q_OBJECT

public:
    PaletteSwatch(LibraryItemPtr item, QSize size = QSize{28,28}, QWidget *parent = nullptr);
    ~PaletteSwatch();

    void setIsAddButton(bool value);
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

signals:
    void selectItem(LibraryItemPtr item);
    void updateItem(LibraryItemPtr item);
    void addItem();

protected:
    void receiveEvent(int eventType, void *source, void *data1, void *data2) override;

private:
    LibraryItemPtr m_item;
    QSize m_size;
    bool m_hover = false;
    bool m_isAddButton = false;
};

struct PaletteGroupData
{
    QString name;
    QList<LibraryItemPtr> items;
};

class PaletteGroup : public QWidget, protected IEventListener
{
    Q_OBJECT

public:
    PaletteGroup(PaletteGroupData *groupData, FolderItemPtr folder, QWidget *parent = nullptr);
    ~PaletteGroup();

signals:
    void selectItem(LibraryItemPtr item);
    void addItem(FolderItemPtr folder);
    void updateItem(LibraryItemPtr item);

protected:
    void receiveEvent(int eventType, void *source, void *data1, void *data2) override;

private slots:
    void addItemClicked();

private:
    void refresh();

    QPushButton *m_labelButton;
    FlowLayout *m_flowLayout;
    PaletteGroupData *m_data;
    FolderItemPtr m_folder;
};



class MiniPalette : public QWidget, protected IEventListener
{
    Q_OBJECT
public:
    explicit MiniPalette(FolderItem *folderItem = nullptr, LibraryTypeId types = ColorItem::TypeId,  QWidget *parent = nullptr);
    ~MiniPalette() override;

    void setFolder(FolderItem *folder);
    FolderItem *folder() const;

   // bool hasHeightForWidth() const override{return true;}
    //int heightForWidth(int w) const override;

signals:
    void selectItem(LibraryItemPtr item);
    void addItem(FolderItemPtr folder);
    void updateSwatch(LibraryItemPtr item);
    void resized();
    void menuOpened();
    void menuClosed();
    void folderChanged();

private slots:
    void addItemClicked();
    void titleClicked();

protected:
    void receiveEvent(int eventType, void *source, void *data1, void *data2) override;

private:
    void rebuildSwatches();


    QVBoxLayout *m_vLayout;
    FolderItem *m_folderItem;
    QPushButton *m_labelButton;
    FlowLayout *m_flowLayout;
    QSize m_swatchSize;
    int m_rowCount = 1;
    LibraryTypeId m_type;
};

} // namespace exo

#endif // MINIPALETTE_H
