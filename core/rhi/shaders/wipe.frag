#version 440

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Wipe {
    vec4 params;   // x = progress, y = angle (radians), z = softness
} u;

layout(binding = 1) uniform sampler2D fromTex;
layout(binding = 2) uniform sampler2D toTex;

void main()
{
    float progress = u.params.x;
    float angle = u.params.y;
    float softness = u.params.z;

    // Coordinate along the wipe direction, normalised to [0,1] across the canvas.
    vec2 dir = vec2(cos(angle), sin(angle));
    float range = abs(dir.x) + abs(dir.y);
    float sMin = min(dir.x, 0.0) + min(dir.y, 0.0);
    float t = (dot(vUV, dir) - sMin) / max(range, 1e-5);

    // Half-width of the transition: the softness band, but never narrower than ~1
    // screen pixel (fwidth) so a hard, rotated edge is still anti-aliased.
    float w = max(softness * 0.5, fwidth(t) * 0.5);

    // Remap progress from [0,1] to [-w, 1+w] so the (full-width) soft band clears
    // the canvas entirely at the extremes — progress 0 is purely "from", progress 1
    // purely "to", with no leftover softened pixels.
    float p = progress * (1.0 + 2.0 * w) - w;
    float m = 1.0 - smoothstep(p - w, p + w, t);   // 1 = show "to"

    fragColor = mix(texture(fromTex, vUV), texture(toTex, vUV), m);
}
