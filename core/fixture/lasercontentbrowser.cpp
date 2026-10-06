#include <QComboBox>
#include <QDir>
#include <QEnterEvent>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QMediaPlayer>
#include <QMouseEvent>
#include <QPainter>
#include <QRegularExpression>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>
#include <QVideoFrame>
#include <QVideoSink>
#include "lasercontentbrowser.h"

namespace photon {

namespace {

constexpr int kTileSize = 128;
constexpr int kColumns = 6;

const QStringList kVideoExtensions = {"mp4", "mov", "m4v"};

// PxxxCyyy - the FB4's own content naming (see LaserPreview).
const QRegularExpression &cueNamePattern()
{
    static const QRegularExpression pattern("^P(\\d+)C(\\d+)$", QRegularExpression::CaseInsensitiveOption);
    return pattern;
}

QString cueName(int t_page, int t_cue)
{
    return QString("P%1C%2").arg(t_page, 3, 10, QChar('0')).arg(t_cue, 3, 10, QChar('0'));
}

QMap<int, QList<int>> scanThumbnails(const QString &t_folder)
{
    QMap<int, QList<int>> cuesByPage;
    const QStringList files = QDir(QDir(t_folder).filePath("thumbs")).entryList({"*.png"}, QDir::Files);
    for(const QString &file : files)
    {
        const auto match = cueNamePattern().match(QFileInfo(file).completeBaseName());
        if(!match.hasMatch())
            continue;
        cuesByPage[match.captured(1).toInt()].append(match.captured(2).toInt());
    }
    for(auto &cues : cuesByPage)
        std::sort(cues.begin(), cues.end());
    return cuesByPage;
}

} // namespace

// ---- Tile ------------------------------------------------------------------

LaserContentTile::LaserContentTile(int t_cue, const QImage &t_thumbnail, bool t_current, QWidget *t_parent)
    : QFrame(t_parent), m_cue(t_cue), m_thumbnail(t_thumbnail), m_current(t_current)
{
    setFixedSize(kTileSize + 8, kTileSize + 24);
    setCursor(Qt::PointingHandCursor);
    setToolTip(QString("Cue %1").arg(t_cue));
}

void LaserContentTile::setFrame(const QImage &t_frame)
{
    m_frame = t_frame;
    update();
}

void LaserContentTile::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    const QRect imageRect(4, 4, kTileSize, kTileSize);
    painter.fillRect(imageRect, Qt::black);
    const QImage &image = m_frame.isNull() ? m_thumbnail : m_frame;
    if(!image.isNull())
    {
        const QSize fitted = image.size().scaled(imageRect.size(), Qt::KeepAspectRatio);
        const QRect target(imageRect.center() - QPoint(fitted.width() / 2, fitted.height() / 2), fitted);
        painter.drawImage(target, image);
    }

    // The capability's current cue keeps a green frame; hover adds a light one.
    if(m_current || m_hovered)
    {
        painter.setPen(QPen(m_current ? QColor(90, 200, 120) : QColor(220, 220, 220), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(imageRect.adjusted(-1, -1, 1, 1));
    }

    painter.setPen(QColor(200, 200, 200));
    painter.drawText(QRect(0, kTileSize + 6, width(), 16), Qt::AlignCenter, QString("Cue %1").arg(m_cue));
}

void LaserContentTile::enterEvent(QEnterEvent *t_event)
{
    m_hovered = true;
    update();
    emit hovered(this, true);
    QFrame::enterEvent(t_event);
}

void LaserContentTile::leaveEvent(QEvent *t_event)
{
    m_hovered = false;
    update();
    emit hovered(this, false);
    QFrame::leaveEvent(t_event);
}

void LaserContentTile::mousePressEvent(QMouseEvent *t_event)
{
    if(t_event->button() == Qt::LeftButton)
        emit clicked(m_cue);
    else
        QFrame::mousePressEvent(t_event);
}

// ---- Browser ---------------------------------------------------------------

LaserContentBrowser::LaserContentBrowser(const QString &t_folder, int t_page, int t_cue, QWidget *t_parent)
    : QDialog(t_parent), m_folder(t_folder), m_currentPage(t_page), m_currentCue(t_cue)
{
    setWindowTitle("Laser Content");
    m_cuesByPage = scanThumbnails(t_folder);

    auto *layout = new QVBoxLayout(this);

    auto *pageRow = new QHBoxLayout;
    m_previousButton = new QToolButton;
    m_previousButton->setArrowType(Qt::LeftArrow);
    m_previousButton->setToolTip("Previous page");
    m_pageCombo = new QComboBox;
    for(auto it = m_cuesByPage.cbegin(); it != m_cuesByPage.cend(); ++it)
        m_pageCombo->addItem(QString("Page %1  (%2 cues)").arg(it.key()).arg(it.value().size()), it.key());
    m_nextButton = new QToolButton;
    m_nextButton->setArrowType(Qt::RightArrow);
    m_nextButton->setToolTip("Next page");
    pageRow->addWidget(m_previousButton);
    pageRow->addWidget(m_pageCombo, 1);
    pageRow->addWidget(m_nextButton);
    layout->addLayout(pageRow);

    m_gridHost = new QWidget;
    m_grid = new QGridLayout(m_gridHost);
    m_grid->setSpacing(4);
    m_grid->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    auto *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setWidget(m_gridHost);
    scroll->setMinimumSize(kColumns * (kTileSize + 12) + 24, 3 * (kTileSize + 28));
    layout->addWidget(scroll, 1);

    if(m_cuesByPage.isEmpty())
        layout->addWidget(new QLabel("No cue thumbnails found in " + QDir(t_folder).filePath("thumbs")));

    m_player = new QMediaPlayer(this);
    m_sink = new QVideoSink(this);
    m_player->setVideoOutput(m_sink);
    m_player->setLoops(QMediaPlayer::Infinite);
    connect(m_sink, &QVideoSink::videoFrameChanged, this, &LaserContentBrowser::videoFrameChanged);

    connect(m_pageCombo, &QComboBox::currentIndexChanged, this, [this](int t_index){
        if(t_index >= 0)
            showPage(m_pageCombo->itemData(t_index).toInt());
    });
    connect(m_previousButton, &QToolButton::clicked, this, [this](){
        m_pageCombo->setCurrentIndex(std::max(0, m_pageCombo->currentIndex() - 1));
    });
    connect(m_nextButton, &QToolButton::clicked, this, [this](){
        m_pageCombo->setCurrentIndex(std::min(m_pageCombo->count() - 1, m_pageCombo->currentIndex() + 1));
    });

    // Open on the capability's current page when it has content.
    const int startIndex = m_pageCombo->findData(t_page);
    m_pageCombo->setCurrentIndex(startIndex >= 0 ? startIndex : 0);
    if(m_shownPage < 0 && m_pageCombo->count() > 0)
        showPage(m_pageCombo->itemData(m_pageCombo->currentIndex()).toInt());
}

LaserContentBrowser::~LaserContentBrowser()
{
    stopPreview();
}

bool LaserContentBrowser::hasContent(const QString &t_folder)
{
    return !t_folder.isEmpty() && !scanThumbnails(t_folder).isEmpty();
}

QString LaserContentBrowser::videoPath(int t_page, int t_cue) const
{
    const QString base = QDir(m_folder).filePath("mp4/" + cueName(t_page, t_cue));
    for(const QString &extension : kVideoExtensions)
    {
        const QString path = base + "." + extension;
        if(QFileInfo::exists(path))
            return path;
    }
    return {};
}

void LaserContentBrowser::showPage(int t_page)
{
    if(t_page == m_shownPage)
        return;
    stopPreview();
    m_shownPage = t_page;

    const int index = m_pageCombo->currentIndex();
    m_previousButton->setEnabled(index > 0);
    m_nextButton->setEnabled(index < m_pageCombo->count() - 1);

    // Replace the previous page's tiles - only one page is ever loaded.
    QLayoutItem *item;
    while((item = m_grid->takeAt(0)) != nullptr)
    {
        delete item->widget();
        delete item;
    }

    const QList<int> cues = m_cuesByPage.value(t_page);
    const QDir thumbs(QDir(m_folder).filePath("thumbs"));
    for(int i = 0; i < cues.size(); ++i)
    {
        const int cue = cues[i];
        const QImage thumbnail(thumbs.filePath(cueName(t_page, cue) + ".png"));
        const bool current = t_page == m_currentPage && cue == m_currentCue;
        auto *tile = new LaserContentTile(cue, thumbnail, current);
        connect(tile, &LaserContentTile::hovered, this, &LaserContentBrowser::tileHovered);
        connect(tile, &LaserContentTile::clicked, this, [this, t_page](int t_cue){
            m_selectedPage = t_page;
            m_selectedCue = t_cue;
            accept();
        });
        m_grid->addWidget(tile, i / kColumns, i % kColumns);
    }
}

void LaserContentBrowser::tileHovered(LaserContentTile *t_tile, bool t_entered)
{
    if(!t_entered)
    {
        if(t_tile == m_previewTile)
            stopPreview();
        return;
    }

    stopPreview();
    const QString path = videoPath(m_shownPage, t_tile->cue());
    if(path.isEmpty())
        return;
    m_previewTile = t_tile;
    m_player->setSource(QUrl::fromLocalFile(path));
    m_player->play();
}

void LaserContentBrowser::videoFrameChanged(const QVideoFrame &t_frame)
{
    if(!m_previewTile || !t_frame.isValid())
        return;
    m_previewTile->setFrame(t_frame.toImage().scaled(kTileSize, kTileSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void LaserContentBrowser::stopPreview()
{
    if(m_player)
    {
        m_player->stop();
        m_player->setSource(QUrl());
    }
    if(m_previewTile)
        m_previewTile->setFrame(QImage());
    m_previewTile = nullptr;
}

} // namespace photon
