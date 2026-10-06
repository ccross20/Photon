#include <cmath>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QMovie>
#include <QPainter>
#include <QVideoFrame>
#include <QVideoSink>
#include <QtMath>
#include "laserpreview.h"
#include "laserimagefilter.h"
#include "data/dmxmatrix.h"
#include "fixture/fixture.h"
#include "fixture/capability/dimmercapability.h"
#include "fixture/capability/lasercapability.h"

namespace photon {

namespace {

using Function = LaserCapability::Function;

// Half the laser's scan field along one axis at 100% size. The FB4's real
// field depends on the projector; this is a typical show-laser value.
constexpr float kFieldHalfAngleDeg = 30.0f;

// Recordings of a black preview aren't truly black (compression noise, UI
// antialiasing); anything below this is treated as no light.
constexpr float kBlackLevel = 0.08f;

const QStringList kExtensions = {"mp4", "mov", "m4v", "gif", "png", "jpg", "jpeg", "webp"};

int rawValue(const Fixture *t_fixture, Function t_function, const DMXMatrix &t_dmx, int t_default)
{
    auto *cap = LaserCapability::find(t_fixture, t_function);
    return cap ? cap->raw(t_dmx) : t_default;
}

// Inverse of LaserCapability::setCentered: -1..1, 0 when the channel is absent.
double centered(const Fixture *t_fixture, Function t_function, const DMXMatrix &t_dmx)
{
    auto *cap = LaserCapability::find(t_fixture, t_function);
    if(!cap)
        return 0.0;
    const double center = cap->is16Bit() ? 32768.0 : 128.0;
    const double top = cap->is16Bit() ? 65535.0 : 255.0;
    const double word = cap->raw(t_dmx);
    return word < center ? word / center - 1.0 : (word - center) / (top - center);
}

double fraction(const Fixture *t_fixture, Function t_function, const DMXMatrix &t_dmx, double t_default)
{
    auto *cap = LaserCapability::find(t_fixture, t_function);
    if(!cap)
        return t_default;
    return cap->raw(t_dmx) / (cap->is16Bit() ? 65535.0 : 255.0);
}

// Continuous rotation speed, -1..1 (+-60 rpm); 0 for "hold" and "cue's own".
double rotationSpeed(const Fixture *t_fixture, const DMXMatrix &t_dmx)
{
    auto *cap = LaserCapability::find(t_fixture, Function::Function_Rotation);
    if(!cap)
        return 0.0;
    int word = cap->raw(t_dmx);
    if(!cap->is16Bit())
        word <<= 8;
    if(word == 0 || word == 32768)
        return 0.0;
    if(word < 32768)
        return (word - 32767) / 32766.0;
    return (word - 32768) / 32767.0;
}

float strobeGate(const Fixture *t_fixture, const DMXMatrix &t_dmx)
{
    const int raw = rawValue(t_fixture, Function::Function_Strobe, t_dmx, 0);
    if(raw <= 0)
        return 1.0f;
    const double hertz = 1.0 + (raw - 1) / 254.0 * 19.0;
    static QElapsedTimer clock;
    if(!clock.isValid())
        clock.start();
    const double phase = std::fmod(clock.elapsed() / 1000.0 * hertz, 1.0);
    return phase < 0.5 ? 1.0f : 0.0f;
}

} // namespace

LaserPreview::LaserPreview(QObject *t_parent) : QObject(t_parent)
{
}

LaserPreview::~LaserPreview()
{
    for(auto &laser : m_lasers)
        close(laser.media);
}

float LaserPreview::fieldTanHalf()
{
    return std::tan(qDegreesToRadians(kFieldHalfAngleDeg));
}

void LaserPreview::beginFrame()
{
    m_seen.clear();
}

void LaserPreview::endFrame()
{
    for(auto it = m_lasers.begin(); it != m_lasers.end(); )
    {
        if(m_seen.contains(it.key()))
        {
            ++it;
            continue;
        }
        close(it.value().media);
        it = m_lasers.erase(it);
    }
}

QString LaserPreview::findMedia(const QString &t_folder, const QString &t_cueName) const
{
    if(t_folder.isEmpty())
        return {};

    // Videos live in an mp4/ subfolder (beside thumbs/, see
    // LaserContentBrowser); older content folders hold them directly.
    for(const QString &subfolder : {QStringLiteral("mp4"), QString()})
    {
        const QDir dir(subfolder.isEmpty() ? t_folder : QDir(t_folder).filePath(subfolder));
        if(!dir.exists())
            continue;
        const QFileInfoList files = dir.entryInfoList(QDir::Files);
        for(const QString &extension : kExtensions)
        {
            for(const QFileInfo &file : files)
            {
                if(file.completeBaseName().compare(t_cueName, Qt::CaseInsensitive) == 0
                   && file.suffix().compare(extension, Qt::CaseInsensitive) == 0)
                    return file.absoluteFilePath();
            }
        }
    }
    return {};
}

void LaserPreview::open(Media &t_media, const QString &t_path)
{
    close(t_media);
    t_media.path = t_path;
    if(t_path.isEmpty())
        return;

    const QString suffix = QFileInfo(t_path).suffix().toLower();
    if(suffix == "mp4" || suffix == "mov" || suffix == "m4v")
    {
        t_media.player = new QMediaPlayer(this);
        t_media.sink = new QVideoSink(this);
        t_media.player->setVideoOutput(t_media.sink);
        t_media.player->setLoops(QMediaPlayer::Infinite);
        std::shared_ptr<QImage> latest = t_media.latest;
        connect(t_media.sink, &QVideoSink::videoFrameChanged, this, [latest](const QVideoFrame &frame){
            // Downscale on arrival: full-size frames are only ever needed at
            // the projection texture's resolution.
            *latest = frame.toImage().scaled(TextureSize, TextureSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        });
        t_media.player->setSource(QUrl::fromLocalFile(t_path));
        t_media.player->play();
    }
    else if(suffix == "gif")
    {
        t_media.movie = new QMovie(t_path, QByteArray(), this);
        t_media.movie->setCacheMode(QMovie::CacheAll);
        t_media.movie->start();
    }
    else
    {
        t_media.still = QImage(t_path).scaled(TextureSize, TextureSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    t_media.speed = 1.0;
}

void LaserPreview::close(Media &t_media)
{
    // deleteLater: a video frame may already be queued for the sink.
    if(t_media.player)
    {
        t_media.player->stop();
        t_media.player->deleteLater();
    }
    if(t_media.sink)
    {
        t_media.sink->disconnect(this);
        t_media.sink->deleteLater();
    }
    if(t_media.movie)
        t_media.movie->deleteLater();
    t_media = Media{};
}

void LaserPreview::setSpeed(Media &t_media, double t_speed)
{
    if(std::abs(t_media.speed - t_speed) < 0.01)
        return;
    t_media.speed = t_speed;

    if(t_media.player)
    {
        if(t_speed <= 0.0)
            t_media.player->pause();
        else
        {
            t_media.player->setPlaybackRate(t_speed);
            t_media.player->play();
        }
    }
    if(t_media.movie)
    {
        t_media.movie->setPaused(t_speed <= 0.0);
        if(t_speed > 0.0)
            t_media.movie->setSpeed(int(std::lround(t_speed * 100.0)));
    }
}

const QImage &LaserPreview::currentImage(const Media &t_media) const
{
    if(t_media.movie)
    {
        *t_media.latest = t_media.movie->currentImage();
        return *t_media.latest;
    }
    if(t_media.player)
        return *t_media.latest;
    return t_media.still;
}

QImage LaserPreview::frameFor(Fixture *t_fixture, const DMXMatrix &t_dmx, float t_dt, float &t_outIntensity)
{
    t_outIntensity = 0.0f;
    m_seen.insert(t_fixture);
    LaserState &laser = m_lasers[t_fixture];

    const int page = rawValue(t_fixture, Function::Function_Page, t_dmx, 0);
    const int cue = rawValue(t_fixture, Function::Function_Cue, t_dmx, 0);
    if(page <= 0 || cue <= 0)
    {
        if(!laser.cueName.isEmpty())
        {
            close(laser.media);
            laser.cueName.clear();
        }
        return {};
    }

    const QString cueName = QString("P%1C%2").arg(page, 3, 10, QChar('0')).arg(cue, 3, 10, QChar('0'));
    if(cueName != laser.cueName)
    {
        laser.cueName = cueName;
        open(laser.media, findMedia(t_fixture->laserPreviewFolder(), cueName));
    }
    if(laser.media.path.isEmpty())
        return {};

    const double cueSpeed = rawValue(t_fixture, Function::Function_CueSpeed, t_dmx, 100) / 100.0;
    setSpeed(laser.media, cueSpeed);

    const QImage &source = currentImage(laser.media);
    if(source.isNull())
        return {};

    double dimmer = 1.0;
    const auto dimmers = t_fixture->findCapability<DimmerCapability *>();
    if(!dimmers.isEmpty())
        dimmer = dimmers.first()->getPercent(t_dmx);

    // The setup profile (Laser Setup dialog) is the laser's installed
    // projection window and output limit; everything the show does happens
    // inside it.
    const Fixture::LaserSetup setup = t_fixture->laserSetup();
    t_outIntensity = float(dimmer * setup.masterIntensity) * strobeGate(t_fixture, t_dmx);
    if(t_outIntensity <= 0.0f)
        return {};

    laser.spin = float(std::fmod(laser.spin + rotationSpeed(t_fixture, t_dmx) * 360.0 * t_dt, 360.0));
    const double angle = fraction(t_fixture, Function::Function_Angle, t_dmx, 0.0) * 360.0 + laser.spin;
    const double zoom = 1.0 + centered(t_fixture, Function::Function_Zoom, t_dmx);
    const double sizeX = std::max(0.0, zoom * (1.0 + centered(t_fixture, Function::Function_SizeX, t_dmx)));
    const double sizeY = std::max(0.0, zoom * (1.0 + centered(t_fixture, Function::Function_SizeY, t_dmx)));
    const double positionX = centered(t_fixture, Function::Function_PositionX, t_dmx);
    const double positionY = centered(t_fixture, Function::Function_PositionY, t_dmx);

    // The square scan field is inscribed in the disk the beam shaders sample,
    // so a full-size frame reaches the disk edge only at its corners.
    const double field = TextureSize / M_SQRT2;
    QImage layout(TextureSize, TextureSize, QImage::Format_ARGB32_Premultiplied);
    layout.fill(Qt::transparent);
    {
        QPainter painter(&layout);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        // Outer: the setup window - the whole scan field is shifted, turned and
        // scaled so that a short setup Y shrinks the content to fit instead of
        // cropping it. The FB4's Geo size is absolute: 100% = full size, 0% =
        // collapsed, negative = mirrored (see Fixture::LaserSetup).
        painter.translate(TextureSize / 2.0 + setup.positionX * field / 2.0,
                          TextureSize / 2.0 - setup.positionY * field / 2.0);
        painter.rotate(setup.rotation);
        painter.scale(setup.sizeX, setup.sizeY);

        // Inner: this frame's live transform, inside that window.
        painter.translate(positionX * field / 2.0, -positionY * field / 2.0);
        painter.rotate(angle);
        painter.scale(sizeX, sizeY);
        const QSizeF fitted = QSizeF(source.size()).scaled(field, field, Qt::KeepAspectRatio);

        // How many output pixels one source pixel covers along the frame's own
        // axes, after every transform above. Below 1 the frame is being shrunk
        // (a short setup size, a small live size), which sub-samples the thin
        // lines unless the image is pre-filtered - they then flicker as the
        // content moves.
        const QTransform world = painter.worldTransform();
        const double ratioX = std::hypot(world.m11(), world.m12()) * fitted.width() / source.width();
        const double ratioY = std::hypot(world.m21(), world.m22()) * fitted.height() / source.height();
        if(ratioX > 0.01 && ratioY > 0.01)
        {
            painter.drawImage(QRectF(QPointF(-fitted.width() / 2.0, -fitted.height() / 2.0), fitted),
                              LaserImage::prefiltered(source, ratioX, ratioY));
        }
    }

    const double mix = fraction(t_fixture, Function::Function_ColorMix, t_dmx, 0.0);
    const double mixRed = fraction(t_fixture, Function::Function_Red, t_dmx, 0.0);
    const double mixGreen = fraction(t_fixture, Function::Function_Green, t_dmx, 0.0);
    const double mixBlue = fraction(t_fixture, Function::Function_Blue, t_dmx, 0.0);

    // Gobo form: hue at full value in rgb, brightness in alpha.
    QImage gobo(TextureSize, TextureSize, QImage::Format_RGBA8888);
    for(int y = 0; y < TextureSize; ++y)
    {
        const QRgb *in = reinterpret_cast<const QRgb *>(layout.constScanLine(y));
        uchar *out = gobo.scanLine(y);
        for(int x = 0; x < TextureSize; ++x, out += 4)
        {
            double r = qRed(in[x]) / 255.0, g = qGreen(in[x]) / 255.0, b = qBlue(in[x]) / 255.0;
            double level = std::max({r, g, b});
            const double lit = std::max(0.0, (level - kBlackLevel) / (1.0 - kBlackLevel));
            if(lit <= 0.0)
            {
                out[0] = out[1] = out[2] = out[3] = 0;
                continue;
            }
            if(mix > 0.0)
            {
                r += (level * mixRed - r) * mix;
                g += (level * mixGreen - g) * mix;
                b += (level * mixBlue - b) * mix;
                level = std::max({r, g, b});
                if(level <= 0.0)
                {
                    out[0] = out[1] = out[2] = out[3] = 0;
                    continue;
                }
            }
            out[0] = uchar(std::lround(r / level * 255.0));
            out[1] = uchar(std::lround(g / level * 255.0));
            out[2] = uchar(std::lround(b / level * 255.0));
            out[3] = uchar(std::lround(lit * 255.0));
        }
    }
    return gobo;
}

} // namespace photon
