#version 440

layout(location = 0) in vec3 vWorld;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Frame {
    mat4 viewProj;
    vec4 lightDir;   // xyz = dir, w = gobo index
    vec4 camPos;     // xyz = camera world position, w = time (seconds)
} frame;

layout(std140, binding = 1) uniform Beam {
    mat4 model;
    vec4 color;      // rgb = emitted color, a = gain
    vec4 apex;       // xyz = cone apex (world), w = second gobo layer (wheel wipe)
    vec4 axisCos;    // xyz = beam axis (world, unit), w = cos(half angle)
    vec4 params;     // x = length, y = gobo layer A, z = gobo rotation, w = color split (-1..1)
    vec4 color2;     // rgb = second color-wheel color (split), w = gobo wipe boundary (-1..1)
    vec4 fadePlane;  // xyz = plane normal (toward apex), w = offset; zero = no fade
    vec4 laser;      // x = laser projection layer (1-based, 0 = none)
} beam;

const int STEPS = 24;

// Laser projections are integrated one laser-texture texel at a time instead
// (see the laser path in main()).
const int LASER_MAX_STEPS = 192;
const float LASER_TEX_SIZE = 256.0;

// Gobo texture array (rgb = transmitted color, a = transmittance).
layout(binding = 2) uniform sampler2DArray goboTex;

// Live laser projections (rgb = hue, a = brightness), see LaserPreview.
layout(binding = 3) uniform sampler2DArray laserTex;

vec2 rotate2(vec2 p, float ang)
{
    float c = cos(ang), s = sin(ang);
    return vec2(c * p.x - s * p.y, s * p.x + c * p.y);
}

void main()
{
    vec3 A     = beam.apex.xyz;
    vec3 d     = beam.axisCos.xyz;
    float cosH = beam.axisCos.w;
    float L    = beam.params.x;
    float gain = beam.color.a;

    // View ray from the camera through this fragment.
    vec3 O = frame.camPos.xyz;
    vec3 v = normalize(vWorld - O);

    // Ray vs. infinite double cone (apex A, axis d, half-angle with cos = cosH).
    vec3 co  = O - A;
    float vd  = dot(v, d);
    float cod = dot(co, d);
    float cos2 = cosH * cosH;

    float a = vd * vd - cos2;
    float b = 2.0 * (vd * cod - dot(v, co) * cos2);
    float c = cod * cod - dot(co, co) * cos2;

    // The stretch [t0, t1] of the view ray that's inside the finite cone: in
    // front of the camera, within the beam's length (0 <= s <= L along the
    // axis), and inside the cone's surface.
    //
    // Inside the double cone is f(t) = a t^2 + b t + c >= 0. Which side of
    // the roots that is depends on a: looking across a beam (a < 0) it's
    // between them, but looking along one - a beam pointed at the camera, or
    // the camera inside it - (a > 0) it's beyond them. Taking "between" in
    // that case sampled the wrong part of the beam, bending a laser's straight
    // sheets like a fisheye.
    const float EPS = 1e-5;

    // Within the length, s = cod + t*vd.
    float t0 = 0.0;
    float t1 = 1e20;
    if (abs(vd) > EPS) {
        float tA = (0.0 - cod) / vd;
        float tB = (L - cod) / vd;
        t0 = max(t0, min(tA, tB));
        t1 = min(t1, max(tA, tB));
    } else if (cod < 0.0 || cod > L) {
        discard;
    }
    if (t1 <= t0)
        discard;

    // Inside the surface. 0 <= s keeps it to the forward nappe, so of the two
    // "beyond" stretches when a > 0, at most one overlaps [t0, t1].
    if (abs(a) < EPS) {
        if (abs(b) < EPS) {
            if (c < 0.0)
                discard;
        } else {
            float r = -c / b;
            if (b > 0.0)
                t0 = max(t0, r);
            else
                t1 = min(t1, r);
        }
    } else {
        float disc = b * b - 4.0 * a * c;
        if (disc < 0.0) {
            if (a < 0.0)
                discard;                // never inside
            // a > 0: inside along its whole length range
        } else {
            float sq = sqrt(disc);
            float r0 = (-b - sq) / (2.0 * a);
            float r1 = (-b + sq) / (2.0 * a);
            if (r0 > r1) { float tmp = r0; r0 = r1; r1 = tmp; }
            if (a < 0.0) {
                t0 = max(t0, r0);
                t1 = min(t1, r1);
            } else {
                float aHi = min(t1, r0);    // before the first root
                float bLo = max(t0, r1);    // after the second
                if (t1 > bLo)
                    t0 = bLo;
                else
                    t1 = aHi;
            }
        }
    }
    if (t1 <= t0)
        discard;

    // tan(half angle) from cosH.
    float tanH = sqrt(max(1.0 / (cosH * cosH) - 1.0, 0.0));

    // Stable frame perpendicular to the axis for the gobo angular coordinate.
    int goboA = int(beam.params.y + 0.5);     // gate-side layers (wheel wipe)
    int goboB = int(beam.apex.w + 0.5);
    float goboSplit = beam.color2.w;
    float goboRot = beam.params.z;
    int laserLayer = int(beam.laser.x + 0.5);
    // Gobo reference frame from the cone's own local axes (model X/Z), so it carries
    // the fixture/prism roll and never flips. A world-up cross product (the old way)
    // is discontinuous when the beam points steeply, which made gobos flip.
    vec3 U = normalize((beam.model * vec4(1.0, 0.0, 0.0, 0.0)).xyz);
    vec3 V = normalize((beam.model * vec4(0.0, 0.0, 1.0, 0.0)).xyz);

    // Representative cross-beam coordinate (along U) at the segment midpoint, for the
    // colour-wheel split. -1..1 across the cone.
    float split = beam.params.w;
    float tmid = 0.5 * (t0 + t1);
    vec3 axm = (O + v * tmid) - A;
    float sm = dot(axm, d);
    float coneRm = max(sm * tanH, 1e-4);
    float xsplit = clamp(dot(axm - d * sm, U) / coneRm, -1.0, 1.0);
    vec3 beamColor = mix(beam.color.rgb, beam.color2.rgb,
                         smoothstep(split - 0.03, split + 0.03, xsplit));

    // March the in-cone segment, accumulating soft scattering density. Per sample
    // we clip to the finite cone (correct nappe, within length, inside radius) so
    // the analytic double-cone roots don't leak.
    float dt = (t1 - t0) / float(STEPS);
    float accum = 0.0;
    vec3  accumGobo = vec3(0.0);   // density-weighted gobo colour (glass tint)
    // Soft-fade against an opaque surface: signed distance is positive on the beam
    // side; samples behind the surface contribute nothing, and the shaft fades over
    // a short band approaching it so the cone/floor intersection has no hard ring.
    bool  hasFade = dot(beam.fadePlane.xyz, beam.fadePlane.xyz) > 1e-6;
    float fadeBand = max(0.08 * L, 0.15);
    // Per-fragment fade by this cone-surface point's distance to the surface. The
    // per-sample fade alone leaves a hard silhouette: a fragment right at the cone/
    // floor intersection still integrates bright samples higher up the chord. Fading
    // the whole fragment as its surface point nears the floor dissolves that edge.
    float fragFade = 1.0;
    if (hasFade) {
        float fsd = dot(beam.fadePlane.xyz, vWorld) + beam.fadePlane.w;
        fragFade = smoothstep(0.0, max(0.18 * L, 0.3), fsd);
    }
    if (laserLayer > 0) {
        // A laser's pattern depends only on the direction from its apex, so a view
        // ray crosses the laser texture along a straight line. Walking that line a
        // texel at a time can't skip a thin bar the way fixed-distance steps do
        // (which left only broken arcs of a sheet), and each sample is weighted by
        // the real length of ray it covers: uv is a projective function of the
        // distance t along the ray, so equal steps in uv are unequal steps in t.
        float cod = dot(O - A, d);
        float vdd = dot(v, d);
        float sMinV = 0.001 * L;
        float ta = t0;
        float tb = t1;
        if (abs(vdd) > 1e-6) {
            float tA = (sMinV - cod) / vdd;
            float tB = (L - cod) / vdd;
            ta = max(ta, min(tA, tB));
            tb = min(tb, max(tA, tB));
        } else if (cod < sMinV || cod > L) {
            tb = ta;
        }
        if (tb > ta) {
            vec3 Xa = O + v * ta - A;
            vec3 Xb = O + v * tb - A;
            float Da = dot(Xa, d);
            float Db = dot(Xb, d);
            vec3 ra = Xa - d * Da;
            vec3 rb = Xb - d * Db;
            vec2 uva = vec2(dot(ra, U), dot(ra, V)) / (Da * tanH);
            vec2 uvb = vec2(dot(rb, U), dot(rb, V)) / (Db * tanH);
            float texels = length(uvb - uva) * 0.5 * LASER_TEX_SIZE;
            int K = int(clamp(ceil(texels) + 1.0, 4.0, float(LASER_MAX_STEPS)));
            for (int j = 0; j < LASER_MAX_STEPS; ++j) {
                if (j >= K)
                    break;
                float lam = (float(j) + 0.5) / float(K);
                float den = Db + lam * (Da - Db);
                float tau = lam * Da / den;
                float dtau = Da * Db / (den * den);
                float t = ta + tau * (tb - ta);
                float s = cod + t * vdd;
                float surfFade = 1.0;
                if (hasFade) {
                    float sd = dot(beam.fadePlane.xyz, O + v * t) + beam.fadePlane.w;
                    if (sd < 0.0)
                        continue;             // behind the opaque surface
                    surfFade = smoothstep(0.0, fadeBand, sd);
                }
                float fl = clamp(1.0 - s / L, 0.0, 1.0) * smoothstep(0.0, 0.06 * L, s);
                vec4 g = textureLod(laserTex, vec3(mix(uva, uvb, lam) * 0.5 + 0.5, float(laserLayer - 1)), 0.0);
                float wgt = fl * g.a * surfFade * (tb - ta) * dtau / float(K);
                accum += wgt;
                accumGobo += g.rgb * wgt;
            }
        }
    } else {
        for (int i = 0; i < STEPS; ++i) {
            float t = t0 + (float(i) + 0.5) * dt;
            vec3 X = O + v * t;
            vec3 ax = X - A;
            float s = dot(ax, d);                 // distance along axis
            if (s < 0.0 || s > L)
                continue;
            float surfFade = 1.0;
            if (hasFade) {
                float sd = dot(beam.fadePlane.xyz, X) + beam.fadePlane.w;
                if (sd < 0.0)
                    continue;                     // behind the opaque surface
                surfFade = smoothstep(0.0, fadeBand, sd);
            }
            float coneR = s * tanH;               // cone radius at this slice
            vec3 rvec = ax - d * s;               // radial vector from axis
            float r = length(rvec);
            float rn = (coneR > 1e-4) ? r / coneR : 1.0;
            if (rn > 1.0)
                continue;
            // Lights get a soft bright core; a laser's brightness is all in its pattern.
            float fr = (laserLayer > 0) ? 1.0 : exp(-3.0 * rn * rn);
            float fl = clamp(1.0 - s / L, 0.0, 1.0);          // fade with distance
            fl *= smoothstep(0.0, 0.06 * L, s);               // soften at the source

            // Pick the gobo layer by cross-beam position (the wheel wipe), then sample.
            float gx = dot(rvec, U) / max(coneR, 1e-4);   // -1..1 across the cone
            int gi = (gx < goboSplit) ? goboA : goboB;
            vec4 g = vec4(1.0);                    // rgb = glass tint, a = transmittance
            if (laserLayer > 0) {
                float theta = atan(dot(rvec, V), dot(rvec, U));
                vec2 luv = vec2(cos(theta), sin(theta)) * rn;
                g = texture(laserTex, vec3(luv * 0.5 + 0.5, float(laserLayer - 1)));
            } else if (gi > 0) {
                float theta = atan(dot(rvec, V), dot(rvec, U));
                vec2 guv = rotate2(vec2(cos(theta), sin(theta)) * rn, goboRot);
                g = texture(goboTex, vec3(guv * 0.5 + 0.5, float(gi - 1)));
            }

            // The gobo modulates the in-air haze but never fully erases it (scattered
            // light keeps the cone glowing), so sparse gobos don't make the beam vanish.
            // (Laser shafts exist only where the pattern is, so no haze floor there.)
            float trans = (laserLayer > 0) ? g.a : mix(0.4, 1.0, g.a);
            float wgt = fr * fl * trans * surfFade * dt;
            accum += wgt;
            accumGobo += g.rgb * wgt;
        }
    }

    // accum is an along-ray density integral (world-distance units). Scale by gain
    // and soft-saturate. (Do NOT divide by L — that made thin-beam chords vanish.)
    // accumGobo / accum is the density-weighted gobo colour, tinting the shaft for
    // glass gobos (metal gobos are white, so this is a no-op for them).
    vec3 goboTint = (accum > 1e-5) ? accumGobo / accum : vec3(1.0);
    float intensity = (1.0 - exp(-accum * gain * 8.0)) * fragFade;
    vec3 col = beamColor * goboTint * intensity;
    fragColor = vec4(col, intensity);                      // premultiplied, additive
}
