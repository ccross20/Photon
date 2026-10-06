#version 440

layout(location = 0) in vec3 vWorld;
layout(location = 1) in vec3 vNormal;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Frame {
    mat4 viewProj;
    vec4 lightDir;   // xyz = dir, w = gobo index
    vec4 camPos;     // xyz = camera, w = time (seconds)
    vec4 ambient;    // rgb = summed ambient lights, w = 1 if the scene has any
    vec4 dirCount;   // x = number of directional lights (0 = use the built-in key)
    vec4 dirLights[8]; // per light: (direction it shines along), (color * intensity)
} frame;

layout(std140, binding = 1) uniform Object {
    mat4 model;
    vec4 color;      // rgb = albedo; a = 1 for a solid (a box's faces), else a flat
                     // surface: 0 = shown both sides, -1 = back hidden, -2 = front hidden
} object;

// Up to 64 spotlights, 6 vec4 each: posRange, dirCosOuter, (color.rgb, goboA),
// (goboRot, goboB, goboSplit, laserLayer), (color2.rgb, split), (frameU.xyz, -).
layout(std140, binding = 2) uniform Lights {
    vec4 countv;
    vec4 data[384];
} lights;

// Gobo texture array: one layer per loaded image (rgb = transmitted color, a = transmittance).
layout(binding = 3) uniform sampler2DArray goboTex;

// Live laser projections (rgb = hue, a = brightness), see LaserPreview.
layout(binding = 4) uniform sampler2DArray laserTex;

vec2 rotate2(vec2 p, float ang)
{
    float c = cos(ang), s = sin(ang);
    return vec2(c * p.x - s * p.y, s * p.x + c * p.y);
}

void main()
{
    vec3 N = normalize(vNormal);

    // Display cull: hide one side of a flat surface (front = the side its
    // normal points out of) so the camera can see through it from there.
    int cull = int(-object.color.a + 0.5);
    if (cull > 0) {
        bool viewingFront = dot(N, frame.camPos.xyz - vWorld) > 0.0;
        if ((cull == 1 && !viewingFront) || (cull == 2 && viewingFront))
            discard;
    }
    vec3 albedo = object.color.rgb;

    // Solids light one-sided (a face turned away from a fixture stays dark)
    // and get a little form shading so an unlit box still reads as a box.
    bool solid = object.color.a > 0.5;

    // The scene's ambient lights, or a faint fallback so unlit surfaces are
    // still visible in a scene without any.
    vec3 result = albedo * (frame.ambient.w > 0.5 ? frame.ambient.rgb : vec3(0.06));
    int dirCount = int(frame.dirCount.x + 0.5);
    if (dirCount > 0) {
        // Directional lights: one-sided on solids, like the spotlights below.
        for (int i = 0; i < dirCount; ++i) {
            vec3 d = frame.dirLights[i * 2].xyz;
            float ndl = solid ? max(dot(N, -d), 0.0) : abs(dot(N, d));
            result += albedo * frame.dirLights[i * 2 + 1].rgb * ndl;
        }
    } else if (solid) {
        result += albedo * 0.10 * max(dot(N, normalize(-frame.lightDir.xyz)), 0.0);
    }

    int count = int(lights.countv.x);
    for (int i = 0; i < count; ++i) {
        vec3  P        = lights.data[i * 6 + 0].xyz;
        float range    = lights.data[i * 6 + 0].w;
        vec3  D        = lights.data[i * 6 + 1].xyz;   // spot axis (apex -> base)
        float cosOuter = lights.data[i * 6 + 1].w;
        vec3  col      = lights.data[i * 6 + 2].xyz;
        int   goboA    = int(lights.data[i * 6 + 2].w + 0.5);
        float goboRot  = lights.data[i * 6 + 3].x;
        int   goboB    = int(lights.data[i * 6 + 3].y + 0.5);
        float goboSplit = lights.data[i * 6 + 3].z;
        int   laserLayer = int(lights.data[i * 6 + 3].w + 0.5);
        vec3  col2     = lights.data[i * 6 + 4].xyz;
        float split    = lights.data[i * 6 + 4].w;
        vec3  frameU   = lights.data[i * 6 + 5].xyz;   // gobo frame reference axis

        vec3 toFrag = vWorld - P;
        float dist = length(toFrag);
        if (dist > range || dist < 1e-4)
            continue;
        vec3 l = toFrag / dist;                  // light -> fragment

        float cosA = dot(l, D);
        if (cosA <= cosOuter)
            continue;                            // outside the cone

        float ang     = acos(clamp(cosA, -1.0, 1.0));
        float halfAng = acos(clamp(cosOuter, -1.0, 1.0));
        float rn = clamp(ang / max(halfAng, 1e-4), 0.0, 1.0);
        // The laser shaft maps its pattern by tan(angle) (a flat projection), so
        // the floor pattern must too or it won't line up with the beams.
        float rnLaser = clamp(tan(ang) / max(tan(halfAng), 1e-4), 0.0, 1.0);
        // Lights: bright core, soft edge. Lasers: the pattern alone decides.
        float shape = (laserLayer > 0) ? 1.0 : smoothstep(1.0, 0.65, rn);

        // Gobo frame from the beam's own local axis (passed in) so the pool's gobo
        // orientation matches the shaft and never flips at steep beam angles.
        vec3 U = normalize(frameU - D * dot(frameU, D));   // orthonormalise against axis
        vec3 V = cross(D, U);
        vec3 pdir = l - D * cosA;                 // perpendicular component
        float theta = atan(dot(pdir, V), dot(pdir, U));

        // Colour-wheel split: pick across the cone (along U) — does not rotate.
        float xsplit = rn * cos(theta);
        vec3 lightCol = mix(col, col2, smoothstep(split - 0.03, split + 0.03, xsplit));

        // Projected gobo: pick the layer by cross-beam position (wheel wipe), then
        // sample the (rotating) gobo texture in the cone disk.
        int gi = (xsplit < goboSplit) ? goboA : goboB;
        vec3 goboRGB = vec3(1.0);
        float goboMask = 1.0;
        if (laserLayer > 0) {
            vec2 luv = vec2(cos(theta), sin(theta)) * rnLaser;
            vec4 g = texture(laserTex, vec3(luv * 0.5 + 0.5, float(laserLayer - 1)));
            goboRGB = g.rgb;
            goboMask = g.a;
        } else if (gi > 0) {
            vec2 guv = rotate2(vec2(cos(theta), sin(theta)) * rn, goboRot);
            vec4 g = texture(goboTex, vec3(guv * 0.5 + 0.5, float(gi - 1)));
            goboRGB = g.rgb;
            goboMask = g.a;
        }

        float ndl = solid ? max(dot(N, -l), 0.0)  // solids: only faces toward the light
                          : abs(dot(N, l));       // flat surfaces are double-sided
        float atten = 1.0 / (1.0 + 0.025 * dist * dist);

        result += albedo * (lightCol * goboRGB) * (shape * goboMask * ndl * atten * 2.6);
    }

    fragColor = vec4(result, 1.0);
}
