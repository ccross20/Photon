#ifndef PHOTON_LASERPREVIEW_H
#define PHOTON_LASERPREVIEW_H

#include <QHash>
#include <QImage>
#include <QObject>
#include <QSet>
#include <memory>

class QMediaPlayer;
class QMovie;
class QVideoSink;

namespace photon {

class DMXMatrix;
class Fixture;

// Turns a laser fixture's live DMX into a projection image for the visualizer,
// using per-cue preview media from Fixture::laserPreviewFolder() - screen
// recordings of the laser software's preview (P001C003.mp4 / .gif) or stills
// for static beams (P001C003.png). The cue's frame is laid out with the DMX
// size/position/rotation/color, and returned in gobo form: rgb = hue at full
// value, a = brightness, with the square field inscribed in the unit disk the
// beam shaders sample.
class LaserPreview : public QObject
{
public:
    static constexpr int TextureSize = 256;

    explicit LaserPreview(QObject *parent = nullptr);
    ~LaserPreview() override;

    // Call around each frame's frameFor() calls; media for lasers not asked
    // about in between (deleted, hidden) is released.
    void beginFrame();
    void endFrame();

    // Null image when the laser shows nothing (page/cue 0, no media, dark).
    // outIntensity is the dimmer x strobe level for the beam.
    QImage frameFor(Fixture *fixture, const DMXMatrix &dmx, float dt, float &outIntensity);

    // Tangent of the field's half-angle along X/Y at full size.
    static float fieldTanHalf();

private:
    struct Media
    {
        QString path;
        QImage still;
        QMovie *movie = nullptr;
        QMediaPlayer *player = nullptr;
        QVideoSink *sink = nullptr;
        // Shared with the sink's frame callback, which must not point into
        // m_lasers (a rehash would move the Media).
        std::shared_ptr<QImage> latest = std::make_shared<QImage>();
        double speed = 1.0;
    };

    struct LaserState
    {
        QString cueName;
        Media media;
        float spin = 0.0f;   // accumulated continuous rotation (degrees)
    };

    QString findMedia(const QString &folder, const QString &cueName) const;
    void open(Media &, const QString &path);
    void close(Media &);
    void setSpeed(Media &, double speed);
    const QImage &currentImage(const Media &) const;

    QHash<Fixture *, LaserState> m_lasers;
    QSet<Fixture *> m_seen;
};

} // namespace photon

#endif // PHOTON_LASERPREVIEW_H
