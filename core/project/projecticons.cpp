#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include "projecticons.h"
#include "project/projectresource.h"
#include "fixture/fixture.h"
#include "color/colorresource.h"
#include "color/gradientresource.h"
#include "color/colorpaletteresource.h"

namespace photon {

namespace {

QIcon pixmapIcon(const QPixmap &t_pixmap)
{
    return QIcon(t_pixmap);
}

// A small rounded, coloured tile with a short glyph in the middle - the
// generic placeholder shape every icon in this file is built from.
QPixmap glyphPixmap(const QString &t_glyph, const QColor &t_color, int t_size = 16)
{
    QPixmap pixmap(t_size, t_size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(t_color);
    painter.drawRoundedRect(QRectF(0.5, 0.5, t_size - 1, t_size - 1), 3, 3);

    painter.setPen(t_color.lightnessF() > 0.6 ? Qt::black : Qt::white);
    QFont font = painter.font();
    font.setPixelSize(t_size - 6);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, t_glyph);

    return pixmap;
}

QPixmap colorSwatchPixmap(const QColor &t_color, int t_size = 16)
{
    QPixmap pixmap(t_size, t_size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QColor(0, 0, 0, 90));
    painter.setBrush(t_color);
    painter.drawRoundedRect(QRectF(0.5, 0.5, t_size - 1, t_size - 1), 3, 3);

    return pixmap;
}

QPixmap gradientSwatchPixmap(const Gradient &t_gradient, int t_size = 16)
{
    QPixmap pixmap(t_size, t_size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath clipPath;
    clipPath.addRoundedRect(QRectF(0.5, 0.5, t_size - 1, t_size - 1), 3, 3);
    painter.setClipPath(clipPath);
    painter.drawImage(pixmap.rect(), t_gradient.toImage(t_size, t_size));
    painter.setClipping(false);

    painter.setPen(QColor(0, 0, 0, 90));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(QRectF(0.5, 0.5, t_size - 1, t_size - 1), 3, 3);

    return pixmap;
}

QPixmap paletteSwatchPixmap(const ColorPalette &t_palette, int t_size = 16)
{
    QPixmap pixmap(t_size, t_size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    if(t_palette.isEmpty())
    {
        painter.setPen(QColor(0, 0, 0, 90));
        painter.setBrush(QColor(120, 120, 120));
        painter.drawRoundedRect(QRectF(0.5, 0.5, t_size - 1, t_size - 1), 3, 3);
        return pixmap;
    }

    QPainterPath clipPath;
    clipPath.addRoundedRect(QRectF(0.5, 0.5, t_size - 1, t_size - 1), 3, 3);
    painter.setClipPath(clipPath);

    // A short strip of swatches, one per palette entry (capped so entries
    // stay visible rather than shrinking to nothing on a big palette).
    const int count = qMin(t_palette.size(), 5);
    const double stripeWidth = double(t_size) / count;
    for(int i = 0; i < count; ++i)
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(t_palette[i]);
        painter.drawRect(QRectF(i * stripeWidth, 0, stripeWidth + 0.5, t_size));
    }

    painter.setClipping(false);

    painter.setPen(QColor(0, 0, 0, 90));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(QRectF(0.5, 0.5, t_size - 1, t_size - 1), 3, 3);

    return pixmap;
}

// Mirrors the coarse category -> model-type inference the visualiser uses
// (plugin-visualizer/rhi/rhirenderer.cpp autoModelType()) for whichever
// fixtures haven't set an explicit Fixture::modelType() override - good
// enough for a placeholder icon even though it's a separate copy.
QString inferredFixtureModelType(Fixture *t_fixture)
{
    const QString explicitType = t_fixture->modelType();
    if(!explicitType.isEmpty())
        return explicitType;

    for(const QString &category : t_fixture->categories())
    {
        const QString lower = category.toLower();
        if(lower.contains("moving head") || lower.contains("scanner"))
            return "mover";
        if(lower.contains("strobe"))
            return "strobe";
        if(lower.contains("blinder"))
            return "blinder";
        if(lower.contains("bar") || lower.contains("matrix"))
            return "bar";
    }
    return "par";
}

QIcon fixtureIcon(Fixture *t_fixture)
{
    const QString modelType = inferredFixtureModelType(t_fixture);

    if(modelType == "mover")
        return pixmapIcon(glyphPixmap("M", QColor(90, 110, 220)));
    if(modelType == "wash")
        return pixmapIcon(glyphPixmap("W", QColor(60, 170, 160)));
    if(modelType == "blinder")
        return pixmapIcon(glyphPixmap("B", QColor(230, 200, 60)));
    if(modelType == "strobe")
        return pixmapIcon(glyphPixmap("S", QColor(220, 70, 70)));
    if(modelType == "bar")
        return pixmapIcon(glyphPixmap("L", QColor(150, 90, 200)));
    if(modelType == "uplight")
        return pixmapIcon(glyphPixmap("U", QColor(90, 150, 90)));
    if(modelType == "beeeye")
        return pixmapIcon(glyphPixmap("E", QColor(200, 130, 60)));

    // "par" and anything unrecognized.
    return pixmapIcon(glyphPixmap("P", QColor(200, 130, 40)));
}

// Scene helper / misc scene-object types, keyed by SceneObject::typeId().
QIcon sceneHelperIcon(const QByteArray &t_typeId)
{
    if(t_typeId == "truss")
        return pixmapIcon(glyphPixmap("T", QColor(130, 130, 130)));
    if(t_typeId == "zone")
        return pixmapIcon(glyphPixmap("Z", QColor(80, 170, 100)));
    if(t_typeId == "axis")
        return pixmapIcon(glyphPixmap("A", QColor(80, 170, 220)));
    if(t_typeId == "direction")
        return pixmapIcon(glyphPixmap(QString::fromUtf8("\xE2\x86\x92"), QColor(200, 90, 170)));
    if(t_typeId == "pointmarker")
        return pixmapIcon(glyphPixmap(QString::fromUtf8("\xE2\x80\xA2"), QColor(220, 190, 60)));
    if(t_typeId == "arrow")
        return pixmapIcon(glyphPixmap(QString::fromUtf8("\xE2\x86\x97"), QColor(210, 110, 60)));
    if(t_typeId == "boundaryoval")
        return pixmapIcon(glyphPixmap("O", QColor(100, 140, 200)));
    if(t_typeId == "boundaryrectangle")
        return pixmapIcon(glyphPixmap("R", QColor(100, 140, 200)));
    if(t_typeId == "linearfalloff")
        return pixmapIcon(glyphPixmap(QString::fromUtf8("\xE2\x89\x88"), QColor(160, 160, 90)));
    if(t_typeId == "group")
        return pixmapIcon(glyphPixmap("G", QColor(140, 140, 150)));
    if(t_typeId == "surface")
        return pixmapIcon(glyphPixmap("S", QColor(80, 130, 180)));
    if(t_typeId == "pixelstrip")
        return pixmapIcon(glyphPixmap(QString::fromUtf8("\xE2\x80\xA6"), QColor(180, 90, 150)));

    return QIcon();
}

} // namespace

QIcon projectResourceIcon(ProjectResource *t_resource)
{
    if(!t_resource)
        return QIcon();

    const QByteArray typeId = t_resource->resourceTypeId();

    if(typeId == "fixture")
    {
        if(auto *fixture = dynamic_cast<Fixture*>(t_resource))
            return fixtureIcon(fixture);
    }
    else if(typeId == "color")
    {
        if(auto *colorResource = dynamic_cast<ColorResource*>(t_resource))
            return pixmapIcon(colorSwatchPixmap(colorResource->color()));
    }
    else if(typeId == "gradient")
    {
        if(auto *gradientResource = dynamic_cast<GradientResource*>(t_resource))
            return pixmapIcon(gradientSwatchPixmap(gradientResource->gradient()));
    }
    else if(typeId == "color-palette")
    {
        if(auto *paletteResource = dynamic_cast<ColorPaletteResource*>(t_resource))
            return pixmapIcon(paletteSwatchPixmap(paletteResource->palette()));
    }
    else
    {
        return sceneHelperIcon(typeId);
    }

    return QIcon();
}

} // namespace photon
