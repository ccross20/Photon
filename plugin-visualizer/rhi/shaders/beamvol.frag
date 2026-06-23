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
} beam;

const int STEPS = 24;

// Gobo texture array (rgb = transmitted color, a = transmittance).
layout(binding = 2) uniform sampler2DArray goboTex;

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

    const float EPS = 1e-5;
    float t0, t1;
    if (abs(a) < EPS) {
        if (abs(b) < EPS)
            discard;
        t0 = t1 = -c / b;
    } else {
        float disc = b * b - 4.0 * a * c;
        if (disc < 0.0)
            discard;
        float sq = sqrt(disc);
        t0 = (-b - sq) / (2.0 * a);
        t1 = (-b + sq) / (2.0 * a);
        if (t0 > t1) { float tmp = t0; t0 = t1; t1 = tmp; }
    }

    t0 = max(t0, 0.0);
    if (t1 <= t0)
        discard;

    // tan(half angle) from cosH.
    float tanH = sqrt(max(1.0 / (cosH * cosH) - 1.0, 0.0));

    // Stable frame perpendicular to the axis for the gobo angular coordinate.
    int goboA = int(beam.params.y + 0.5);     // gate-side layers (wheel wipe)
    int goboB = int(beam.apex.w + 0.5);
    float goboSplit = beam.color2.w;
    float goboRot = beam.params.z;
    vec3 up = abs(d.y) > 0.99 ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0);
    vec3 U = normalize(cross(up, d));
    vec3 V = cross(d, U);

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
        float fr = exp(-3.0 * rn * rn);        // soft bright core
        float fl = clamp(1.0 - s / L, 0.0, 1.0);          // fade with distance
        fl *= smoothstep(0.0, 0.06 * L, s);               // soften at the source

        // Pick the gobo layer by cross-beam position (the wheel wipe), then sample.
        float gx = dot(rvec, U) / max(coneR, 1e-4);   // -1..1 across the cone
        int gi = (gx < goboSplit) ? goboA : goboB;
        vec4 g = vec4(1.0);                    // rgb = glass tint, a = transmittance
        if (gi > 0) {
            float theta = atan(dot(rvec, V), dot(rvec, U));
            vec2 guv = rotate2(vec2(cos(theta), sin(theta)) * rn, goboRot);
            g = texture(goboTex, vec3(guv * 0.5 + 0.5, float(gi - 1)));
        }

        // The gobo modulates the in-air haze but never fully erases it (scattered
        // light keeps the cone glowing), so sparse gobos don't make the beam vanish.
        float trans = mix(0.4, 1.0, g.a);
        float wgt = fr * fl * trans * surfFade * dt;
        accum += wgt;
        accumGobo += g.rgb * wgt;
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
