#include "pixelsource.h"
#include "sequence/sequence.h"
#include "opengl/openglframebuffer.h"
#include "pixel/canvas.h"

namespace photon {


struct tRGB{
    tRGB(int r = 0,int g = 0,int b = 0):r(r),g(g),b(b){}
    int r,g,b;
};

struct RGBW{
    RGBW(int r = 0,int g = 0,int b = 0,int w = 0):r(r),g(g),b(b),w(w){}
    int r,g,b,w;
};

class PixelSource::Impl
{
public:
    RGBW GetRgbwFromRgb(tRGB rgb, int colorTemp);
};

static QMap<int, tRGB> getKelvinTable()
{
    return QMap<int, tRGB>{
            {1000, tRGB(255, 56, 0)},
            {1100, tRGB(255, 71, 0)},
            {1200, tRGB(255, 83, 0)},
            {1300, tRGB(255, 93, 0)},
            {1400, tRGB(255, 101, 0)},
            {1500, tRGB(255, 109, 0)},
            {1600, tRGB(255, 115, 0)},
            {1700, tRGB(255, 121, 0)},
            {1800, tRGB(255, 126, 0)},
            {1900, tRGB(255, 131, 0)},
            {2000, tRGB(255, 138, 18)},
            {2100, tRGB(255, 142, 33)},
            {2200, tRGB(255, 147, 44)},
            {2300, tRGB(255, 152, 54)},
            {2400, tRGB(255, 157, 63)},
            {2500, tRGB(255, 161, 72)},
            {2600, tRGB(255, 165, 79)},
            {2700, tRGB(255, 169, 87)},
            {2800, tRGB(255, 173, 94)},
            {2900, tRGB(255, 177, 101)},
            {3000, tRGB(255, 180, 107)},
            {3100, tRGB(255, 184, 114)},
            {3200, tRGB(255, 187, 120)},
            {3300, tRGB(255, 190, 126)},
            {3400, tRGB(255, 193, 132)},
            {3500, tRGB(255, 196, 137)},
            {3600, tRGB(255, 199, 143)},
            {3700, tRGB(255, 201, 148)},
            {3800, tRGB(255, 204, 153)},
            {3900, tRGB(255, 206, 159)},
            {4000, tRGB(255, 209, 163)},
            {4100, tRGB(255, 211, 168)},
            {4200, tRGB(255, 213, 173)},
            {4300, tRGB(255, 215, 177)},
            {4400, tRGB(255, 217, 182)},
            {4500, tRGB(255, 219, 186)},
            {4600, tRGB(255, 221, 190)},
            {4700, tRGB(255, 223, 194)},
            {4800, tRGB(255, 225, 198)},
            {4900, tRGB(255, 227, 202)},
            {5000, tRGB(255, 228, 206)},
            {5100, tRGB(255, 230, 210)},
            {5200, tRGB(255, 232, 213)},
            {5300, tRGB(255, 233, 217)},
            {5400, tRGB(255, 235, 220)},
            {5500, tRGB(255, 236, 224)},
            {5600, tRGB(255, 238, 227)},
            {5700, tRGB(255, 239, 230)},
            {5800, tRGB(255, 240, 233)},
            {5900, tRGB(255, 242, 236)},
            {6000, tRGB(255, 243, 239)},
            {6100, tRGB(255, 244, 242)},
            {6200, tRGB(255, 245, 245)},
            {6300, tRGB(255, 246, 247)},
            {6400, tRGB(255, 248, 251)},
            {6500, tRGB(255, 249, 253)},
            {6600, tRGB(254, 249, 255)},
            {6700, tRGB(252, 247, 255)},
            {6800, tRGB(249, 246, 255)},
            {6900, tRGB(247, 245, 255)},
            {7000, tRGB(245, 243, 255)},
            {7100, tRGB(243, 242, 255)},
            {7200, tRGB(240, 241, 255)},
            {7300, tRGB(239, 240, 255)},
            {7400, tRGB(237, 239, 255)},
            {7500, tRGB(235, 238, 255)},
            {7600, tRGB(233, 237, 255)},
            {7700, tRGB(231, 236, 255)},
            {7800, tRGB(230, 235, 255)},
            {7900, tRGB(228, 234, 255)},
            {8000, tRGB(227, 233, 255)},
            {8100, tRGB(225, 232, 255)},
            {8200, tRGB(224, 231, 255)},
            {8300, tRGB(222, 230, 255)},
            {8400, tRGB(221, 230, 255)},
            {8500, tRGB(220, 229, 255)},
            {8600, tRGB(218, 229, 255)},
            {8700, tRGB(217, 227, 255)},
            {8800, tRGB(216, 227, 255)},
            {8900, tRGB(215, 226, 255)},
            {9000, tRGB(214, 225, 255)},
            {9100, tRGB(212, 225, 255)},
            {9200, tRGB(211, 224, 255)},
            {9300, tRGB(210, 223, 255)},
            {9400, tRGB(209, 223, 255)},
            {9500, tRGB(208, 222, 255)},
            {9600, tRGB(207, 221, 255)},
            {9700, tRGB(207, 221, 255)},
            {9800, tRGB(206, 220, 255)},
            {9900, tRGB(205, 220, 255)},
            {10000, tRGB(207, 218, 255)},
            {10100, tRGB(207, 218, 255)},
            {10200, tRGB(206, 217, 255)},
            {10300, tRGB(205, 217, 255)},
            {10400, tRGB(204, 216, 255)},
            {10500, tRGB(204, 216, 255)},
            {10600, tRGB(203, 215, 255)},
            {10700, tRGB(202, 215, 255)},
            {10800, tRGB(202, 214, 255)},
            {10900, tRGB(201, 214, 255)},
            {11000, tRGB(200, 213, 255)},
            {11100, tRGB(200, 213, 255)},
            {11200, tRGB(199, 212, 255)},
            {11300, tRGB(198, 212, 255)},
            {11400, tRGB(198, 212, 255)},
            {11500, tRGB(197, 211, 255)},
            {11600, tRGB(197, 211, 255)},
            {11700, tRGB(197, 210, 255)},
            {11800, tRGB(196, 210, 255)},
            {11900, tRGB(195, 210, 255)},
            {12000, tRGB(195, 209, 255)}};
}


RGBW PixelSource::Impl::GetRgbwFromRgb(tRGB rgb, int colorTemp) {
  //Get the maximum between R, G, and B
  uint8_t r = rgb.r;
  uint8_t g = rgb.g;
  uint8_t b = rgb.b;

  colorTemp = std::round(colorTemp / 100.0) * 100;

  if(colorTemp < 1000)
      colorTemp = 1000;
  else if(colorTemp > 12000)
      colorTemp = 12000;

  auto kelvinRGB = getKelvinTable()[colorTemp];

  // These values are what the 'white' value would need to
  // be to get the corresponding color value.
  double whiteValueForRed = r * 255.0 / kelvinRGB.r;
  double whiteValueForGreen = g * 255.0 / kelvinRGB.g;
  double whiteValueForBlue = b * 255.0 / kelvinRGB.b;

  // Set the white value to the highest it can be for the given color
  // (without over saturating any channel - thus the minimum of them).
  double minWhiteValue = std::min(whiteValueForRed,
                             std::min(whiteValueForGreen,
                                 whiteValueForBlue));
  uint8_t Wo = (minWhiteValue <= 255 ? (uint8_t) minWhiteValue : 255);

  // The rest of the channels will just be the original value minus the
  // contribution by the white channel.
  uint8_t Ro = (uint8_t)(r - minWhiteValue * kelvinRGB.r / 255);
  uint8_t Go = (uint8_t)(g - minWhiteValue * kelvinRGB.g / 255);
  uint8_t Bo = (uint8_t)(b - minWhiteValue * kelvinRGB.b / 255);

  return RGBW(Ro, Go, Bo, Wo);
}

PixelSource::PixelSource() : m_impl(new Impl)
{


}

PixelSource::~PixelSource()
{
    delete m_impl;
}

void PixelSource::setFormat(PixelFormat)
{

}

PixelSource::PixelFormat PixelSource::format() const
{
    return Format_RGBW;
}


QVector3D PixelSource::boundingVector() const
{
    return QVector3D{};
}

int PixelSource::dmxOffset() const
{
    return 0;
}

int PixelSource::dmxSize() const
{
    return 1;
}

int PixelSource::universe() const
{
    return 1;
}






void PixelSource::process(ProcessContext &t_context, const QTransform &t_transform) const
{
    int u = universe()-1;
    int channel = dmxOffset();

    //qDebug() << "Write to dmx " << u << channel;
    if(!t_context.image)
        return;

    for(auto it = positions().cbegin(); it != positions().cend(); it++)
    {
        auto ptF = t_transform.map((*it));
        ptF.setX(ptF.x() * t_context.canvas->width());
        ptF.setY(ptF.y() * t_context.canvas->height());

        auto pt = ptF.toPoint();
        QRgb color;
        if(pt.x() < 0 || pt.x() >= t_context.canvas->width() || pt.y() < 0 || pt.y() >= t_context.canvas->height())
        {
            color = 0;
        }
        else
        {
            color = t_context.image->pixel(pt);
        }

        auto qc = QColor::fromRgb(color);

        auto rgbw = m_impl->GetRgbwFromRgb(tRGB{qc.red(), qc.green(), qc.blue()},2700);

        t_context.dmxMatrix.setValue(u, channel++, rgbw.r);
        t_context.dmxMatrix.setValue(u, channel++, rgbw.g);
        t_context.dmxMatrix.setValue(u, channel++, rgbw.b);
        t_context.dmxMatrix.setValue(u, channel++, rgbw.w);
    }
}

void PixelSource::readFromJson(const QJsonObject &, const LoadContext &)
{

}

void PixelSource::writeToJson(QJsonObject &) const
{

}

} // namespace photon
