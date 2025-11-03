#include "fillnoiseeffect.h"
#include "channel/parameter/numberchannelparameter.h"
#include "channel/parameter/point2channelparameter.h"
#include "channel/parameter/colorchannelparameter.h"
#include "channel/parameter/optionchannelparameter.h"
#include "util/gilconcurrent.h"
#include "util/utils.h"
#include "pixel/canvas.h"
#include "opengl/openglshader.h"
#include "opengl/openglplane.h"

using namespace boost::gil;
namespace photon {

FillNoiseEffect::FillNoiseEffect()
{

}

void FillNoiseEffect::init()
{
    addChannelParameter(new OptionChannelParameter("type",QStringList{"Value","Value Fractal","Perlin","Perlin Fractal","Simplex","Simplex Fractal","Cellular","White Noise","Cubic","Cublic Fractal"}));
    addChannelParameter(new OptionChannelParameter("interpolation",QStringList{"Linear","Hermite","Quintic"}));
    addChannelParameter(new Point2ChannelParameter("origin",QPointF{0,0}));
    addChannelParameter(new Point2ChannelParameter("scale",QPointF{.5,.5}));
    addChannelParameter(new NumberChannelParameter("seed"));
    addChannelParameter(new NumberChannelParameter("frequency", .1,0,100));
    addChannelParameter(new NumberChannelParameter("speed"));
    addChannelParameter(new NumberChannelParameter("direction"));
    addChannelParameter(new ColorChannelParameter("colorA", Qt::white));
    addChannelParameter(new ColorChannelParameter("colorB", Qt::black));
}

void FillNoiseEffect::initializeContext(QOpenGLContext *t_context, Canvas *t_canvas)
{
    m_texture = new OpenGLTexture;
    m_texture->resize(t_context, QImage::Format::Format_ARGB32_Premultiplied, t_canvas->width(), t_canvas->height());

    m_plane = new OpenGLPlane(t_context, bounds_d{-1,-1,1,1}, false);
    m_shader = new OpenGLShader(t_context, ":/resources/shader/BasicTextureVertex.vert",
                                ":/resources/shader/texture.frag");
    m_shader->bind(t_context);

}

void FillNoiseEffect::canvasResized(QOpenGLContext *t_context, Canvas *t_canvas)
{
    m_texture->resize(t_context, QImage::Format::Format_ARGB32_Premultiplied, t_canvas->width(), t_canvas->height());
}

void FillNoiseEffect::evaluate(CanvasEffectEvaluationContext &t_context)
{
    //m_noise.noise()
    //t_context.canvasImage

    int w = t_context.canvas->width();
    int h = t_context.canvas->height();

    QImage image{w,h,QImage::Format_ARGB32_Premultiplied};

    NoiseGenerator noise;

    QColor colorA = t_context.channelValues["colorA"].value<QColor>();
    QColor colorB = t_context.channelValues["colorB"].value<QColor>();

    colorA.setAlphaF(colorA.alphaF() * t_context.strength);
    colorB.setAlphaF(colorB.alphaF() * t_context.strength);

    noise.setOffset(t_context.channelValues["origin"].value<QPointF>());
    noise.setScale(t_context.channelValues["scale"].value<QPointF>());
    noise.setFrequency(t_context.channelValues["frequency"].toDouble());
    noise.setNoiseType(static_cast<NoiseGenerator::NoiseType>(t_context.channelValues["type"].toInt()));
    noise.setInterpolation(static_cast<NoiseGenerator::Interpolation>(t_context.channelValues["interpolation"].toInt()));
    noise.setSeed(t_context.channelValues["seed"].toInt());
    int pitch = 4;
    int scanWidth = w * pitch;

    auto view = interleaved_view(w,h,reinterpret_cast<bgra8_ptr_t>(image.bits()),scanWidth);

    bounds_i outBounds{0,0,w,h};

    concurrentViewTransform<bgra8_view_t>(view, outBounds,[&noise, &colorA, &colorB](const bgra8_view_t &output, const bounds_i &bounds)
    {
        for (std::ptrdiff_t y=0; y<output.height(); ++y) {
            typename bgra8_view_t::x_iterator it=output.row_begin(y);
            for (std::ptrdiff_t x=0; x<output.width(); ++x, ++it)
            {
                float greyVal = (noise.noise1D(x + bounds.x1, y + bounds.y1)+1)/2.0;
                if(greyVal < 0.0f)
                    greyVal = 0.0;
                else if(greyVal > 1.0f)
                    greyVal = 1.0f;

                QColor c = blendColors(colorA, colorB, greyVal);

                (*it)[0] = c.red();
                (*it)[1] = c.green();
                (*it)[2] = c.blue();
                (*it)[3] = c.alpha();
            }
        }
    }
    );

    m_texture->loadRaster(t_context.openglContext, &image);
    m_shader->bind(t_context.openglContext);
    m_shader->setTexture("tex",m_texture->handle());

    m_plane->draw();

}

ClipEffectInformation FillNoiseEffect::info()
{
    ClipEffectInformation toReturn([](){return new FillNoiseEffect;});
    toReturn.name = "Fill Noise";
    toReturn.id = "photon.clip.effect.fill-noise";
    toReturn.categories.append("Fill");

    return toReturn;
}

} // namespace photon
