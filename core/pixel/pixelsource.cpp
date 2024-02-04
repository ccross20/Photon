#include "pixelsource.h"
#include "sequence/sequence.h"

namespace photon {


struct RGB{
    RGB(int r = 0,int g = 0,int b = 0):r(r),g(g),b(b){}
    int r,g,b;
};

struct RGBW{
    RGBW(int r = 0,int g = 0,int b = 0,int w = 0):r(r),g(g),b(b),w(w){}
    int r,g,b,w;
};

class PixelSource::Impl
{
public:
    RGBW GetRgbwFromRgb(RGB rgb, int colorTemp);
};

static QMap<int, RGB> getKelvinTable()
{
    return QMap<int, RGB>{
            {1000, RGB(255, 56, 0)},
            {1100, RGB(255, 71, 0)},
            {1200, RGB(255, 83, 0)},
            {1300, RGB(255, 93, 0)},
            {1400, RGB(255, 101, 0)},
            {1500, RGB(255, 109, 0)},
            {1600, RGB(255, 115, 0)},
            {1700, RGB(255, 121, 0)},
            {1800, RGB(255, 126, 0)},
            {1900, RGB(255, 131, 0)},
            {2000, RGB(255, 138, 18)},
            {2100, RGB(255, 142, 33)},
            {2200, RGB(255, 147, 44)},
            {2300, RGB(255, 152, 54)},
            {2400, RGB(255, 157, 63)},
            {2500, RGB(255, 161, 72)},
            {2600, RGB(255, 165, 79)},
            {2700, RGB(255, 169, 87)},
            {2800, RGB(255, 173, 94)},
            {2900, RGB(255, 177, 101)},
            {3000, RGB(255, 180, 107)},
            {3100, RGB(255, 184, 114)},
            {3200, RGB(255, 187, 120)},
            {3300, RGB(255, 190, 126)},
            {3400, RGB(255, 193, 132)},
            {3500, RGB(255, 196, 137)},
            {3600, RGB(255, 199, 143)},
            {3700, RGB(255, 201, 148)},
            {3800, RGB(255, 204, 153)},
            {3900, RGB(255, 206, 159)},
            {4000, RGB(255, 209, 163)},
            {4100, RGB(255, 211, 168)},
            {4200, RGB(255, 213, 173)},
            {4300, RGB(255, 215, 177)},
            {4400, RGB(255, 217, 182)},
            {4500, RGB(255, 219, 186)},
            {4600, RGB(255, 221, 190)},
            {4700, RGB(255, 223, 194)},
            {4800, RGB(255, 225, 198)},
            {4900, RGB(255, 227, 202)},
            {5000, RGB(255, 228, 206)},
            {5100, RGB(255, 230, 210)},
            {5200, RGB(255, 232, 213)},
            {5300, RGB(255, 233, 217)},
            {5400, RGB(255, 235, 220)},
            {5500, RGB(255, 236, 224)},
            {5600, RGB(255, 238, 227)},
            {5700, RGB(255, 239, 230)},
            {5800, RGB(255, 240, 233)},
            {5900, RGB(255, 242, 236)},
            {6000, RGB(255, 243, 239)},
            {6100, RGB(255, 244, 242)},
            {6200, RGB(255, 245, 245)},
            {6300, RGB(255, 246, 247)},
            {6400, RGB(255, 248, 251)},
            {6500, RGB(255, 249, 253)},
            {6600, RGB(254, 249, 255)},
            {6700, RGB(252, 247, 255)},
            {6800, RGB(249, 246, 255)},
            {6900, RGB(247, 245, 255)},
            {7000, RGB(245, 243, 255)},
            {7100, RGB(243, 242, 255)},
            {7200, RGB(240, 241, 255)},
            {7300, RGB(239, 240, 255)},
            {7400, RGB(237, 239, 255)},
            {7500, RGB(235, 238, 255)},
            {7600, RGB(233, 237, 255)},
            {7700, RGB(231, 236, 255)},
            {7800, RGB(230, 235, 255)},
            {7900, RGB(228, 234, 255)},
            {8000, RGB(227, 233, 255)},
            {8100, RGB(225, 232, 255)},
            {8200, RGB(224, 231, 255)},
            {8300, RGB(222, 230, 255)},
            {8400, RGB(221, 230, 255)},
            {8500, RGB(220, 229, 255)},
            {8600, RGB(218, 229, 255)},
            {8700, RGB(217, 227, 255)},
            {8800, RGB(216, 227, 255)},
            {8900, RGB(215, 226, 255)},
            {9000, RGB(214, 225, 255)},
            {9100, RGB(212, 225, 255)},
            {9200, RGB(211, 224, 255)},
            {9300, RGB(210, 223, 255)},
            {9400, RGB(209, 223, 255)},
            {9500, RGB(208, 222, 255)},
            {9600, RGB(207, 221, 255)},
            {9700, RGB(207, 221, 255)},
            {9800, RGB(206, 220, 255)},
            {9900, RGB(205, 220, 255)},
            {10000, RGB(207, 218, 255)},
            {10100, RGB(207, 218, 255)},
            {10200, RGB(206, 217, 255)},
            {10300, RGB(205, 217, 255)},
            {10400, RGB(204, 216, 255)},
            {10500, RGB(204, 216, 255)},
            {10600, RGB(203, 215, 255)},
            {10700, RGB(202, 215, 255)},
            {10800, RGB(202, 214, 255)},
            {10900, RGB(201, 214, 255)},
            {11000, RGB(200, 213, 255)},
            {11100, RGB(200, 213, 255)},
            {11200, RGB(199, 212, 255)},
            {11300, RGB(198, 212, 255)},
            {11400, RGB(198, 212, 255)},
            {11500, RGB(197, 211, 255)},
            {11600, RGB(197, 211, 255)},
            {11700, RGB(197, 210, 255)},
            {11800, RGB(196, 210, 255)},
            {11900, RGB(195, 210, 255)},
            {12000, RGB(195, 209, 255)}};
}


RGBW PixelSource::Impl::GetRgbwFromRgb(RGB rgb, int colorTemp) {
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

    return;
    //qDebug() << "Write to dmx " << u << channel;

    for(auto it = positions().cbegin(); it != positions().cend(); it++)
    {
        auto ptF = t_transform.map((*it));
        ptF.setX(ptF.x() * t_context.canvasImage->width());
        ptF.setY(ptF.y() * t_context.canvasImage->height());

        auto pt = ptF.toPoint();
        QRgb color;
        if(pt.x() < 0 || pt.x() >= t_context.canvasImage->width() || pt.y() < 0 || pt.y() >= t_context.canvasImage->height())
        {
            color = 0;
        }
        else
        {
            color = t_context.canvasImage->pixel(pt);
            //qDebug() << pt << QColor::fromRgb(color);
        }

        auto qc = QColor::fromRgb(color);

        auto rgbw = m_impl->GetRgbwFromRgb(RGB{qc.red(), qc.green(), qc.blue()},2700);

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
