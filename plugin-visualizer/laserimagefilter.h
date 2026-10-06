#ifndef PHOTON_LASERIMAGEFILTER_H
#define PHOTON_LASERIMAGEFILTER_H

#include <algorithm>
#include <cmath>
#include <QImage>

namespace photon {
namespace LaserImage {

// Grows bright features by rx/ry pixels (per-channel box maximum), so a thin
// laser line survives being shrunk instead of averaging away to nothing.
inline QImage dilated(const QImage &t_in, int t_rx, int t_ry)
{
    const QImage src = t_in.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const int w = src.width(), h = src.height();
    QImage horizontal(w, h, src.format());
    QImage out(w, h, src.format());

    auto channelMax = [](QRgb a, QRgb b) {
        return qRgba(std::max(qRed(a), qRed(b)), std::max(qGreen(a), qGreen(b)),
                     std::max(qBlue(a), qBlue(b)), std::max(qAlpha(a), qAlpha(b)));
    };

    for(int y = 0; y < h; ++y)
    {
        const QRgb *in = reinterpret_cast<const QRgb *>(src.constScanLine(y));
        QRgb *o = reinterpret_cast<QRgb *>(horizontal.scanLine(y));
        for(int x = 0; x < w; ++x)
        {
            QRgb m = in[x];
            for(int k = std::max(0, x - t_rx); k <= std::min(w - 1, x + t_rx); ++k)
                m = channelMax(m, in[k]);
            o[x] = m;
        }
    }
    for(int y = 0; y < h; ++y)
    {
        QRgb *o = reinterpret_cast<QRgb *>(out.scanLine(y));
        for(int x = 0; x < w; ++x)
        {
            QRgb m = reinterpret_cast<const QRgb *>(horizontal.constScanLine(y))[x];
            for(int k = std::max(0, y - t_ry); k <= std::min(h - 1, y + t_ry); ++k)
                m = channelMax(m, reinterpret_cast<const QRgb *>(horizontal.constScanLine(k))[x]);
            o[x] = m;
        }
    }
    return out;
}

// `t_source` readied for being drawn at t_ratioX/t_ratioY final pixels per
// source pixel. Where that shrinks the image, thin features are widened to the
// size of the footprint they'll be averaged over, then area-averaged down to
// roughly the final size - no aliasing from skipped samples (which flickers as
// the frame moves) and no dimming of thin lines (which a plain average does).
// The result is drawn at about 1:1, so it needs no further minification.
inline QImage prefiltered(const QImage &t_source, double t_ratioX, double t_ratioY)
{
    if(t_ratioX >= 1.0 && t_ratioY >= 1.0)
        return t_source;

    constexpr int kMaxRadius = 12;
    const int rx = t_ratioX < 1.0 ? std::min(kMaxRadius, int(std::floor(0.5 / std::max(t_ratioX, 1e-3)))) : 0;
    const int ry = t_ratioY < 1.0 ? std::min(kMaxRadius, int(std::floor(0.5 / std::max(t_ratioY, 1e-3)))) : 0;

    const QImage widened = (rx > 0 || ry > 0) ? dilated(t_source, rx, ry) : t_source;
    const int w = std::max(1, int(std::lround(t_source.width() * std::min(1.0, t_ratioX))));
    const int h = std::max(1, int(std::lround(t_source.height() * std::min(1.0, t_ratioY))));
    return widened.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

} // namespace LaserImage
} // namespace photon

#endif // PHOTON_LASERIMAGEFILTER_H
