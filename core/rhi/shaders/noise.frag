#version 440

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Noise {
    vec4 params;    // x,y = offset, z = scale, w = time (animation)
    vec4 params2;   // x = mode, y = complexity (octaves), z = warp, w = lacunarity
    vec4 params3;   // x = seed
} u;

const int MAX_OCTAVES = 8;

float hash13(vec3 p)
{
    p = fract(p * 0.1031);
    p += dot(p, p.yzx + 33.33);
    return fract((p.x + p.y) * p.z);
}

vec3 hash33(vec3 p)
{
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
             dot(p, vec3(269.5, 183.3, 246.1)),
             dot(p, vec3(113.5, 271.9, 124.6)));
    return fract(sin(p) * 43758.5453123);
}

vec2 hash22(vec2 p)
{
    p = vec2(dot(p, vec2(127.1, 311.7)), dot(p, vec2(269.5, 183.3)));
    return fract(sin(p) * 43758.5453123);
}

// 3D value noise (z is driven by time to animate), range ~[0,1].
// x,y use smoothstep for spatial smoothness, but z uses LINEAR interpolation:
// smoothstep's derivative is zero at integer boundaries, which would make the
// animation pause-and-accelerate once per unit of time.
float valueNoise(vec3 p)
{
    vec3 i = floor(p);
    vec3 f = fract(p);
    vec2 w = f.xy * f.xy * (3.0 - 2.0 * f.xy);
    float wz = f.z;
    return mix(
        mix(mix(hash13(i + vec3(0,0,0)), hash13(i + vec3(1,0,0)), w.x),
            mix(hash13(i + vec3(0,1,0)), hash13(i + vec3(1,1,0)), w.x), w.y),
        mix(mix(hash13(i + vec3(0,0,1)), hash13(i + vec3(1,0,1)), w.x),
            mix(hash13(i + vec3(0,1,1)), hash13(i + vec3(1,1,1)), w.x), w.y),
        wz);
}

// Fractal (fBm): sum octaves of value noise; "complexity" = octave count.
float fbm(vec3 p, int octaves, float lacunarity)
{
    float amp = 0.5;
    float sum = 0.0;
    float norm = 0.0;
    for (int i = 0; i < MAX_OCTAVES; ++i) {
        if (i >= octaves)
            break;
        sum += amp * valueNoise(p);
        norm += amp;
        p *= lacunarity;
        amp *= 0.5;
    }
    return sum / max(norm, 1e-4);
}

// Cellular (Worley): distance to the nearest feature point.
float worley(vec3 p)
{
    vec3 i = floor(p);
    vec3 f = fract(p);
    float minDist = 1.0;
    for (int x = -1; x <= 1; ++x)
    for (int y = -1; y <= 1; ++y)
    for (int z = -1; z <= 1; ++z) {
        vec3 g = vec3(x, y, z);
        vec3 o = hash33(i + g);
        vec3 r = g + o - f;
        minDist = min(minDist, dot(r, r));
    }
    return sqrt(minDist);
}

// Flow noise: 2D gradient noise whose lattice gradients ROTATE over time, so
// features swirl/rotate in place instead of crossfading between static slices.
vec2 gradRot(vec2 cell, float t)
{
    vec2 h = hash22(cell);
    float baseAng = 6.2831853 * h.x;
    float speed = 0.6 + 0.8 * h.y;        // per-cell rate variation
    float dir = h.x < 0.5 ? 1.0 : -1.0;   // and direction
    float ang = baseAng + t * 6.2831853 * 0.15 * speed * dir;
    return vec2(cos(ang), sin(ang));
}

float flowNoise(vec2 p, float t)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 w = f * f * (3.0 - 2.0 * f);
    float n = mix(
        mix(dot(gradRot(i + vec2(0,0), t), f - vec2(0,0)),
            dot(gradRot(i + vec2(1,0), t), f - vec2(1,0)), w.x),
        mix(dot(gradRot(i + vec2(0,1), t), f - vec2(0,1)),
            dot(gradRot(i + vec2(1,1), t), f - vec2(1,1)), w.x), w.y);
    return n * 0.5 + 0.5;
}

float flowFbm(vec2 p, float t, int octaves)
{
    float amp = 0.5;
    float sum = 0.0;
    float norm = 0.0;
    for (int i = 0; i < MAX_OCTAVES; ++i) {
        if (i >= octaves)
            break;
        sum += amp * flowNoise(p, t);
        norm += amp;
        p *= 2.0;
        t *= 1.6;   // finer octaves evolve faster
        amp *= 0.5;
    }
    return sum / max(norm, 1e-4);
}

// Divergence-free (curl) warp from an animated flow-noise potential. Because the
// displacement is the curl of a scalar field, it is rotational — the domain
// swirls/circulates rather than being pushed forward and back (which is what a
// plain gradient/value warp does as the noise crosses its midpoint).
vec2 curlWarp(vec2 uv, float t, float warp, int octaves)
{
    if (warp <= 0.001)
        return vec2(0.0);
    const float e = 0.1;
    float dPdx = (flowFbm(uv + vec2(e, 0.0), t, octaves)
                - flowFbm(uv - vec2(e, 0.0), t, octaves)) / (2.0 * e);
    float dPdy = (flowFbm(uv + vec2(0.0, e), t, octaves)
                - flowFbm(uv - vec2(0.0, e), t, octaves)) / (2.0 * e);
    return warp * vec2(dPdy, -dPdx);
}

void main()
{
    // Seed shifts the sampled region of the (infinite) noise field, giving a
    // different pattern per seed across all modes.
    float seed = u.params3.x;
    vec2 seedOff = fract(sin(vec2(seed * 12.9898, seed * 78.233)) * 43758.5453) * 256.0;
    float seedT = fract(sin(seed * 39.425) * 12345.678) * 256.0;

    vec2 uv = (vUV + u.params.xy) * u.params.z + seedOff;
    float t = u.params.w + seedT;
    float mode = u.params2.x;
    int octaves = int(u.params2.y + 0.5);
    float warp = u.params2.z;
    float lacunarity = u.params2.w;

    float n;
    if (mode < 0.5) {                 // Value
        vec3 p = vec3(uv + curlWarp(uv, t, warp, octaves), t);
        n = valueNoise(p);
    } else if (mode < 1.5) {          // Fractal
        vec3 p = vec3(uv + curlWarp(uv, t, warp, octaves), t);
        n = fbm(p, octaves, lacunarity);
    } else if (mode < 2.5) {          // Cellular
        n = worley(vec3(uv, t));
    } else {                          // Flow
        n = flowFbm(uv, t, octaves);
    }

    n = clamp(n, 0.0, 1.0);
    fragColor = vec4(vec3(n), 1.0);
}
