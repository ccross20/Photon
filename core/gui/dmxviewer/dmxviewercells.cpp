#include <QPainter>
#include <QRandomGenerator>
#include <QMouseEvent>
#include "dmxviewercells.h"
#include "data/dmxmatrix.h"

namespace photon {

class DMXViewerCells::Impl
{
public:

    class CellData
    {
    public:
        CellData(int w, int h):rect(0,0,w,h){

        }
        QRect rect;
        uchar value = 0;
        bool needsRedraw = true;
    };


    Impl(DMXViewerCells *t_facade);
    void hoverCell(int t_cell);
    DMXMatrix dmxMatrix;
    QVector<CellData> cells;
    QVector<QPixmap> pixmaps;
    int margin = 5;
    int spacing = 2;
    int cellWidth = 30;
    int cellHeight = 20;
    int hoveringCell = -1;
    DMXViewerCells *facade;
};

DMXViewerCells::Impl::Impl(DMXViewerCells *t_facade):facade(t_facade)
{
    QPen pen(Qt::white);
    pen.setWidth(0);


    QPen boxPen(QColor(150,150,150));
    boxPen.setWidth(0);

    QFont font;
    font.setPixelSize(10);

    QPixmap pix(cellWidth, cellHeight);
    pix.fill(Qt::transparent);
    pixmaps.fill(pix, 512);
    cells.fill(CellData(cellWidth, cellHeight),512);

    for(int i = 0; i < 512; ++i)
    {


        QPainter painter{&pixmaps[i]};
        painter.setFont(font);
        painter.setBrush(Qt::NoBrush);

        painter.setPen(boxPen);
        painter.drawRect(0,0, cellWidth-1, cellHeight-1);
        painter.setPen(pen);
        painter.drawText(QRect(0,0, cellWidth, cellHeight),Qt::AlignCenter, QString::number(i+1));


    }
}

void DMXViewerCells::Impl::hoverCell(int t_cell)
{
    if(hoveringCell == t_cell)
        return;

    if(hoveringCell >= 0)
    {
        cells[hoveringCell].needsRedraw = true;
        facade->update(cells[hoveringCell].rect);
    }

    if(t_cell >= 0 && t_cell < 512)
    {
        hoveringCell = t_cell;
        cells[hoveringCell].needsRedraw = true;
        facade->update(cells[hoveringCell].rect);
    }
    else
        hoveringCell = -1;

}

DMXViewerCells::DMXViewerCells(QWidget *parent)
    : QWidget{parent},m_impl(new Impl(this))
{

    setMouseTracking(true);
}

DMXViewerCells::~DMXViewerCells()
{
    delete m_impl;
}

void DMXViewerCells::leaveEvent(QEvent *t_e)
{
    QWidget::leaveEvent(t_e);
    m_impl->hoverCell(-1);
}

void DMXViewerCells::mouseMoveEvent(QMouseEvent *t_event)
{
    QWidget::mouseMoveEvent(t_event);

    QPoint pt = t_event->pos() - QPoint(m_impl->margin, m_impl->margin);

    int column = pt.x() / (m_impl->cellWidth + m_impl->spacing);
    int row = pt.y() / (m_impl->cellHeight + m_impl->spacing);
    int columnsPerRow = std::floor(width() / (m_impl->cellWidth + m_impl->spacing));

    m_impl->hoverCell(row * columnsPerRow + column);
}

void DMXViewerCells::setData(const photon::DMXMatrix &t_matrix)
{
    m_impl->dmxMatrix = t_matrix;

    auto cellIt = m_impl->cells.begin();

    for(auto it = t_matrix.channels[0].cbegin(); it != t_matrix.channels[0].cend() && cellIt != m_impl->cells.end(); ++it)
    {
        if((*cellIt).value != *it)
            (*cellIt).needsRedraw = true;
        (*cellIt).value = *it;
        ++cellIt;
    }

    update();
}

void DMXViewerCells::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);
    QPainter painter{this};

    int w = m_impl->cellWidth;
    int h = m_impl->cellHeight;
    int x = m_impl->margin;
    int y = m_impl->margin;
    int spacing = m_impl->spacing;
    for(int i = 0; i < 512; ++i)
    {
        DMXViewerCells::Impl::CellData &cell = m_impl->cells[i];

        painter.fillRect(x,y,w * cell.value/255.0f,h, QColor(Qt::green).darker(150));

        if(cell.needsRedraw && i == m_impl->hoveringCell)
            painter.drawPixmap(x,y, m_impl->pixmaps[cell.value]);
        else
            painter.drawPixmap(x,y, m_impl->pixmaps[i]);

        cell.rect = QRect(x,y,w,h);

        if(cell.needsRedraw && i == m_impl->hoveringCell)
        {
            painter.save();
            painter.setPen(QPen(Qt::white, 0));
            painter.drawRect(x,y,w-1,h-1);
            painter.restore();
        }

        x += w + spacing;

        if(x > width() - w)
        {
            x = m_impl->margin;
            y+= h + spacing;
        }
    }

}

} // namespace photon
