#include <QColor>
#include "colorcapability.h"

namespace photon {


class ColorCapability::Impl
{
public:
    QVector<ColorIntensityCapability*> channels;
    bool hasWhite = false;
    bool hasAmber = false;
    bool hasLime = false;
};


ColorCapability::ColorCapability(const QVector<ColorIntensityCapability*> &t_channels) : FixtureCapability(DMXRange(),Capability_Color),m_impl(new Impl)
{
    m_impl->channels = t_channels;
    setChannel(t_channels[0]->channel());

    for(auto channel : m_impl->channels)
    {
        switch (channel->type()) {
        case Capability_Red:
            break;
        case Capability_Green:
            break;
        case Capability_Blue:
            break;
        case Capability_Lime:
            m_impl->hasLime = true;
            break;
        case Capability_Amber:
            m_impl->hasAmber = true;
            break;
        case Capability_White:
            m_impl->hasWhite = true;
            break;
            /*
        case Capability_UV:
            channel->setPercent(c.blueF(), t_matrix, t_blend);
            break;
*/
        default:
            break;
        }

    }
}

ColorCapability::~ColorCapability()
{
    delete m_impl;
}

struct R_G_B_A_L_W {
    uint8_t R;
    uint8_t G;
    uint8_t B;
    uint8_t A; // Amber
    uint8_t L; // Lime
    uint8_t W; // White
};

// Define the input structure
struct R_G_B {
    uint8_t R;
    uint8_t G;
    uint8_t B;
};

const float W_FACTOR = 1.0f; // White contribution relative to min(R, G, B)
const float A_FACTOR = 0.0f; // Amber influence on Red/Green (Amber is R+G-ish)
const float L_FACTOR = 0.0f; // Lime influence on Green/Blue (Lime is G+B-ish, but near Green)
//const float A_FACTOR = 0.5f; // Amber influence on Red/Green (Amber is R+G-ish)
//const float L_FACTOR = 0.5f; // Lime influence on Green/Blue (Lime is G+B-ish, but near Green)

/**
 * @brief Converts an RGB color to R G B Amber Lime White (RGBLAW)
 * @param input_rgb The input R G B color structure (0-255 per channel).
 * @return The 6-channel output structure.
 */
R_G_B_A_L_W rgb_to_rgbalw(const R_G_B& input_rgb, bool hasAmber, bool hasLime, bool hasWhite) {
    // Start with floating point values for accurate intermediate calculations
    float r = static_cast<float>(input_rgb.R);
    float g = static_cast<float>(input_rgb.G);
    float b = static_cast<float>(input_rgb.B);

    R_G_B_A_L_W output = {0, 0, 0, 0, 0, 0};

    // 1. White Extraction (Achromatic Component)
    // The White channel replaces the minimum shared component of R, G, and B
    float min_rgb = 0.f;

    // Calculate the White channel value
    if(hasWhite)
    {
        min_rgb = std::min({r, g, b});
        float w_val = min_rgb * W_FACTOR;
        output.W = static_cast<uint8_t>(std::min(w_val, 255.0f));
    }


    // Subtract the white component from R, G, B
    r -= min_rgb;
    g -= min_rgb;
    b -= min_rgb;

    if(hasAmber)
    {
        // 2. Amber Extraction (Yellowish Component - mostly Red and Green)
        // Amber is between Red and Green. We extract the shared remaining R and G.
        float min_rg = std::min(r, g);

        // Calculate the Amber channel value
        float a_val = min_rg * A_FACTOR;
        output.A = static_cast<uint8_t>(std::min(a_val, 255.0f));

        // Subtract the amber component (scaled by its color properties)
        // A pure Amber LED is roughly a mix of R and G in terms of color perception.
        r -= min_rg;
        g -= min_rg;
    }

    if(hasLime)
    {
        // 3. Lime Extraction (Greenish-Yellow Component - mostly Green)
        // Lime is near Green. We can use it to boost the remaining green component.
        // A common use of Lime is to increase Green efficiency/brightness.
        // Let's use it on the *remaining* pure green for efficiency/brighter lime.
        float l_val = g * L_FACTOR;
        output.L = static_cast<uint8_t>(std::min(l_val, 255.0f));

        // Subtract the lime component
        g -= l_val / L_FACTOR; // Remove the full 'g' amount used for Lime
    }

    // 4. Set Remaining R, G, B
    // The final R, G, B are the components that couldn't be efficiently replaced by W, A, or L.
    output.R = static_cast<uint8_t>(std::min(r, 255.0f));
    output.G = static_cast<uint8_t>(std::min(g, 255.0f));
    output.B = static_cast<uint8_t>(static_cast<float>(input_rgb.B) - min_rgb); // Remaining Blue is just original B - White

    return output;
}

void ColorCapability::setColor(const QColor &t_color, DMXMatrix &t_matrix, double t_blend) const
{
    /*
    QColor c = t_color.toCmyk();
    for(auto channel : m_impl->channels)
    {
        switch (channel->type()) {
        case Capability_Cyan:
            channel->setPercent(c.cyanF(), t_matrix, t_blend);
            break;
        case Capability_Magenta:
            channel->setPercent(c.magentaF(), t_matrix, t_blend);
            break;
        case Capability_Yellow:
            channel->setPercent(c.yellowF(), t_matrix, t_blend);
            break;
        default:
            break;
        }

    }
*/

    QColor c = t_color.toRgb();
    R_G_B rgb;
    rgb.R = c.red();
    rgb.G = c.green();
    rgb.B = c.blue();

    //auto ledColors = rgb_to_rgbalw(rgb, m_impl->hasAmber, m_impl->hasLime, m_impl->hasWhite);
    auto ledColors = rgb_to_rgbalw(rgb, false,false, m_impl->hasWhite);


    for(auto channel : m_impl->channels)
    {
        switch (channel->type()) {
        case Capability_Red:
            channel->setPercent(ledColors.R/255.0, t_matrix, t_blend);
            break;
        case Capability_Green:
            channel->setPercent(ledColors.G/255.0, t_matrix, t_blend);
            break;
        case Capability_Blue:
            channel->setPercent(ledColors.B/255.0, t_matrix, t_blend);
            break;
        case Capability_Lime:
            channel->setPercent(ledColors.L/255.0, t_matrix, t_blend);
            break;
        case Capability_Amber:
            channel->setPercent(ledColors.A/255.0, t_matrix, t_blend);
            break;
        case Capability_White:
            channel->setPercent(ledColors.W/255.0, t_matrix, t_blend);
            break;
            /*
        case Capability_UV:
            channel->setPercent(c.blueF(), t_matrix, t_blend);
            break;
*/
        default:
            break;
        }

    }

}

QColor ColorCapability::getColor(const DMXMatrix &t_matrix) const
{
    /*
    QColor color;
    float c = 1.0f,m = 1.0f,y= 1.0f, k = 0.0f;

    for(auto channel : m_impl->channels)
    {
        switch (channel->type()) {
        case Capability_Cyan:
            c = channel->getPercent(t_matrix);
            break;
        case Capability_Magenta:
            m = channel->getPercent(t_matrix);
            break;
        case Capability_Yellow:
            y = channel->getPercent(t_matrix);
            break;
        default:
            break;
        }

    }
    color.setCmykF(c,m,y,k);
    return color;
*/

    QColor color;
    float r = 1.0f,g = 1.0f,b= 1.0f;

    for(auto channel : m_impl->channels)
    {
        switch (channel->type()) {
        case Capability_Red:
            r = channel->getPercent(t_matrix);
            break;
        case Capability_Green:
            g = channel->getPercent(t_matrix);
            break;
        case Capability_Blue:
            b = channel->getPercent(t_matrix);
            break;
        default:
            break;
        }

    }
    color.setRgbF(r,g,b);
    return color;
}

} // namespace photon
