#version 440

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

const int MAX_STOPS = 16;

layout(std140, binding = 0) uniform GradMap {
    vec4 params;             // x = channel (0 lum,1 r,2 g,3 b,4 a), y = invert, z = mix, w = stop count
    vec4 positions[4];       // stop positions, packed 4 per vec4
    vec4 colors[MAX_STOPS];  // stop colours (straight rgba)
} u;

layout(binding = 1) uniform sampler2D sourceTex;

float stopPos(int i)
{
    return u.positions[i / 4][i % 4];
}

vec4 gradientColor(float t)
{
    int count = int(u.params.w);
    if (count <= 0)
        return vec4(0.0);
    if (count == 1)
        return u.colors[0];

    if (t <= stopPos(0))
        return u.colors[0];

    for (int i = 1; i < count; ++i)
    {
        float p1 = stopPos(i);
        if (t <= p1)
        {
            float p0 = stopPos(i - 1);
            float f = (t - p0) / max(p1 - p0, 1e-6);
            return mix(u.colors[i - 1], u.colors[i], f);
        }
    }
    return u.colors[count - 1];
}

void main()
{
    vec4 src = texture(sourceTex, vUV);

    int channel = int(u.params.x + 0.5);
    float t;
    if (channel == 0)      t = dot(src.rgb, vec3(0.2126, 0.7152, 0.0722));   // luminance
    else if (channel == 1) t = src.r;
    else if (channel == 2) t = src.g;
    else if (channel == 3) t = src.b;
    else                   t = src.a;

    if (u.params.y > 0.5)
        t = 1.0 - t;
    t = clamp(t, 0.0, 1.0);

    vec4 mapped = gradientColor(t);
    // Keep the source's transparency so masked/empty areas stay empty.
    mapped.a *= src.a;

    fragColor = mix(src, mapped, u.params.z);
}
