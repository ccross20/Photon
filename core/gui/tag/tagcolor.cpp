#include "tagcolor.h"

namespace photon {

QColor tagColor(const QString &t_tag)
{
    // qHash rather than QString's own hash: stable across Qt versions/builds,
    // since a chip's color must stay the same tag-to-tag, run-to-run.
    const uint hash = qHash(t_tag.trimmed().toLower());

    const int hue = static_cast<int>(hash % 360);
    // Fixed saturation/value rather than deriving them from the hash too -
    // varying brightness alongside hue makes some tags hard to read and
    // others washed out. A consistent, moderately saturated, moderately
    // bright swatch keeps every tag equally legible.
    return QColor::fromHsv(hue, 150, 210);
}

QColor tagTextColor(const QColor &t_background)
{
    // Standard perceptual luminance weighting (ITU-R BT.601).
    const double luminance = (0.299 * t_background.red()
                             + 0.587 * t_background.green()
                             + 0.114 * t_background.blue()) / 255.0;
    return luminance > 0.6 ? QColor(Qt::black) : QColor(Qt::white);
}

} // namespace photon
