#version 440

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

const int MAX_STOPS = 16;

layout(std140, binding = 0) uniform Grad {
    vec4 header;              // x = type (0 linear, 1 radial, 2 angular), y = angle (rad), z = stop count
    vec4 shape;              // xy = center, z = radius
    vec4 positions[4];       // stop positions, packed 4 per vec4
    vec4 colors[MAX_STOPS];  // stop colours (straight, non-premultiplied rgba)
} u;

float stopPos(int i)
{
    return u.positions[i / 4][i % 4];
}

vec4 gradientColor(float t)
{
    int count = int(u.header.z);
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
    int type = int(u.header.x);
    float t;

    if (type == 1)
    {
        // Radial: distance from centre, normalised by radius.
        float r = max(u.shape.z, 1e-6);
        t = clamp(distance(vUV, u.shape.xy) / r, 0.0, 1.0);
    }
    else if (type == 2)
    {
        // Angular / conic: angle around the centre, offset by the base angle.
        vec2 d = vUV - u.shape.xy;
        float a = atan(d.y, d.x) - u.header.y;
        t = fract(a / (2.0 * 3.14159265359));
    }
    else
    {
        // Linear: project onto the wipe direction, normalised across the canvas.
        vec2 dir = vec2(cos(u.header.y), sin(u.header.y));
        float range = abs(dir.x) + abs(dir.y);
        float sMin = min(dir.x, 0.0) + min(dir.y, 0.0);
        t = clamp((dot(vUV, dir) - sMin) / max(range, 1e-6), 0.0, 1.0);
    }

    fragColor = gradientColor(t);
}
