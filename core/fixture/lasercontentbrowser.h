#ifndef PHOTON_LASERCONTENTBROWSER_H
#define PHOTON_LASERCONTENTBROWSER_H

#include <QDialog>
#include <QFrame>
#include <QImage>
#include <QMap>
#include <QPointer>
#include "photon-global.h"

class QComboBox;
class QGridLayout;
class QMediaPlayer;
class QToolButton;
class QVideoFrame;
class QVideoSink;

namespace photon {

// One cue in the browser: its thumbnail, or the playing video while hovered.
class LaserContentTile : public QFrame
{
    Q_OBJECT
public:
    LaserContentTile(int cue, const QImage &thumbnail, bool selected, QWidget *parent = nullptr);

    int cue() const { return m_cue; }
    // The browser's current selection - drawn with a green frame.
    void setSelected(bool selected);
    // Shown in place of the thumbnail while the cue's video plays; a null
    // image goes back to the thumbnail.
    void setFrame(const QImage &frame);

signals:
    void hovered(photon::LaserContentTile *tile, bool entered);
    void clicked(int cue);
    void doubleClicked(int cue);

protected:
    void paintEvent(QPaintEvent *) override;
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseDoubleClickEvent(QMouseEvent *) override;

private:
    int m_cue;
    QImage m_thumbnail;
    QImage m_frame;
    bool m_selected;
    bool m_hovered = false;
};

// Picks a laser cue by eye. Reads a laser content folder laid out as
//   <folder>/thumbs/PxxxCyyy.png  - a still of each cue (128x128)
//   <folder>/mp4/PxxxCyyy.mp4     - the cue's recorded preview
// and shows one page of cues at a time (only that page's thumbnails are
// loaded). Hovering a cue plays its video in the tile. Clicking one selects
// it and reports it straight away (contentSelected), so the laser can be
// previewed while browsing; double-clicking selects and closes. OK keeps the
// selection; Cancel reports the page/cue it opened with again and closes.
class PHOTONCORE_EXPORT LaserContentBrowser : public QDialog
{
    Q_OBJECT
public:
    LaserContentBrowser(const QString &contentFolder, int page, int cue, QWidget *parent = nullptr);
    ~LaserContentBrowser();

    // Whether a folder has any cue thumbnails to browse.
    static bool hasContent(const QString &contentFolder);

    int selectedPage() const { return m_selectedPage; }
    int selectedCue() const { return m_selectedCue; }

    void reject() override;

signals:
    // A cue was chosen - by a click, or the original restored by Cancel.
    void contentSelected(int page, int cue);

private:
    void showPage(int page);
    void select(int page, int cue);
    void tileHovered(LaserContentTile *tile, bool entered);
    void videoFrameChanged(const QVideoFrame &frame);
    void stopPreview();
    QString videoPath(int page, int cue) const;

    QString m_folder;
    QMap<int, QList<int>> m_cuesByPage;   // page -> sorted cue numbers
    int m_originalPage;                    // the capability's page/cue on open
    int m_originalCue;
    int m_shownPage = -1;
    int m_selectedPage;
    int m_selectedCue;

    QComboBox *m_pageCombo = nullptr;
    QToolButton *m_previousButton = nullptr;
    QToolButton *m_nextButton = nullptr;
    QWidget *m_gridHost = nullptr;
    QGridLayout *m_grid = nullptr;

    // One player for the whole browser, playing whichever tile is hovered.
    QMediaPlayer *m_player = nullptr;
    QVideoSink *m_sink = nullptr;
    QPointer<LaserContentTile> m_previewTile;
};

} // namespace photon

#endif // PHOTON_LASERCONTENTBROWSER_H
