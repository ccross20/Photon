#version 440

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Shape {
    vec4 a;      // position.xy, size.xy
    vec4 b;      // rotation (radians), roundness, softness, aspect
    vec4 c;      // shape index, sides, stroke width
    vec4 color;  // fill rgba
    vec4 stroke; // stroke rgba
} u;

// --- 2D signed distance functions (iquilezles.org/articles/distfunctions2d) ---

float sdCircle(vec2 p, float r)
{
    return length(p) - r;
}

float sdRoundBox(vec2 p, vec2 b, float r)
{
    vec2 q = abs(p) - b + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

float sdEquilateralTriangle(vec2 p, float r)
{
    const float k = sqrt(3.0);
    p.x = abs(p.x) - r;
    p.y = p.y + r / k;
    if (p.x + k * p.y > 0.0)
        p = vec2(p.x - k * p.y, -k * p.x - p.y) / 2.0;
    p.x -= clamp(p.x, -2.0 * r, 0.0);
    return -length(p) * sign(p.y);
}

float sdPentagon(vec2 p, float r)
{
    const vec3 k = vec3(0.809016994, 0.587785252, 0.726542528);
    p.x = abs(p.x);
    p -= 2.0 * min(dot(vec2(-k.x, k.y), p), 0.0) * vec2(-k.x, k.y);
    p -= 2.0 * min(dot(vec2( k.x, k.y), p), 0.0) * vec2( k.x, k.y);
    p -= vec2(clamp(p.x, -r * k.z, r * k.z), r);
    return length(p) * sign(p.y);
}

float sdHexagon(vec2 p, float r)
{
    const vec3 k = vec3(-0.866025404, 0.5, 0.577350269);
    p = abs(p);
    p -= 2.0 * min(dot(k.xy, p), 0.0) * k.xy;
    p -= vec2(clamp(p.x, -k.z * r, k.z * r), r);
    return length(p) * sign(p.y);
}

float sdStar(vec2 p, float r, int n, float m)
{
    float an = 3.141593 / float(n);
    float en = 3.141593 / m;
    vec2 acs = vec2(cos(an), sin(an));
    vec2 ecs = vec2(cos(en), sin(en));

    float bn = mod(atan(p.x, p.y), 2.0 * an) - an;
    p = length(p) * vec2(cos(bn), abs(sin(bn)));
    p -= r * acs;
    p += ecs * clamp(-dot(p, ecs), 0.0, r * acs.y / ecs.y);
    return length(p) * sign(p.x);
}

void main()
{
    // Centre at position, correct for aspect (so circles stay round), rotate.
    vec2 p = vUV - u.a.xy;
    p.x *= u.b.w;
    float rot = u.b.x;
    float cs = cos(-rot), sn = sin(-rot);
    p = mat2(cs, -sn, sn, cs) * p;

    int shape = int(u.c.x + 0.5);
    float r = u.a.z;              // size.x as radius for radial shapes
    vec2 boxB = u.a.zw;           // size.xy as half-extents for the box
    float round = u.b.y;
    int sides = max(int(u.c.y + 0.5), 2);

    float d;
    if (shape == 0)
        d = sdCircle(p, r);
    else if (shape == 1)
        d = sdRoundBox(p, boxB, clamp(round, 0.0, min(boxB.x, boxB.y)));
    else if (shape == 2)
        d = sdEquilateralTriangle(p, r);
    else if (shape == 3)
        d = sdPentagon(p, r);
    else if (shape == 4)
        d = sdHexagon(p, r);
    else
        d = sdStar(p, r, sides, clamp(mix(2.5, float(sides), clamp(round, 0.0, 1.0)), 2.0, float(sides)));

    // Analytic anti-aliasing from the distance's screen-space gradient; softness
    // widens the edge.
    float aa = max(fwidth(d), u.b.z);
    float fillCoverage = 1.0 - smoothstep(-aa, aa, d);

    // Stroke: a band of the given width centred on the shape's boundary (d == 0).
    float halfW = u.c.z * 0.5;
    float strokeCoverage = halfW > 0.0 ? (1.0 - smoothstep(halfW - aa, halfW + aa, abs(d))) : 0.0;

    // Composite stroke over fill (straight alpha), both over a transparent exterior.
    float fillA = u.color.a * fillCoverage;
    float strokeA = u.stroke.a * strokeCoverage;
    float outA = strokeA + fillA * (1.0 - strokeA);
    vec3 outRGB = outA > 0.0
        ? (u.stroke.rgb * strokeA + u.color.rgb * fillA * (1.0 - strokeA)) / outA
        : vec3(0.0);

    fragColor = vec4(outRGB, outA);
}
