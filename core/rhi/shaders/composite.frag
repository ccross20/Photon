#version 440

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Composite {
    vec4 xform0;   // position.xy, scale.xy
    vec4 xform1;   // rotation (radians), origin.xy, alpha
    vec4 params;   // x = blend mode
} u;

layout(binding = 1) uniform sampler2D baseTex;
layout(binding = 2) uniform sampler2D topTex;

vec3 blend(vec3 b, vec3 s, float mode)
{
    if (mode < 0.5)  return s;                              // Normal
    if (mode < 1.5)  return b + s;                          // Add
    if (mode < 2.5)  return b * s;                          // Multiply
    if (mode < 3.5)  return 1.0 - (1.0 - b) * (1.0 - s);    // Screen
    if (mode < 4.5) {                                       // Overlay
        return mix(2.0 * b * s,
                   1.0 - 2.0 * (1.0 - b) * (1.0 - s),
                   step(vec3(0.5), b));
    }
    if (mode < 5.5)  return b - s;                          // Subtract
    if (mode < 6.5)  return abs(b - s);                     // Difference
    if (mode < 7.5)  return max(b, s);                      // Lighten
    return min(b, s);                                       // Darken
}

void main()
{
    vec4 baseColor = texture(baseTex, vUV);

    // Inverse-transform the output uv into the top texture's uv (top is placed at
    // position, scaled and rotated about origin).
    vec2 pos = u.xform0.xy;
    vec2 scale = u.xform0.zw;
    float rot = u.xform1.x;
    vec2 origin = u.xform1.yz;
    float alpha = u.xform1.w;

    vec2 p = vUV - pos - origin;
    float c = cos(-rot), s = sin(-rot);
    p = mat2(c, -s, s, c) * p;
    p /= max(scale, vec2(1e-4));
    p += origin;

    vec4 topColor = texture(topTex, clamp(p, 0.0, 1.0));

    // Anti-alias the top's rectangular edge: instead of a hard in/out clip, feather
    // coverage across ~1 screen pixel using the uv derivatives (fwidth). The
    // transition is centred ON the edge (0.5 at the boundary), so a top that exactly
    // fills the canvas stays fully opaque at the border (no 1px base bleed-through)
    // while rotated/scaled edges still smooth out.
    vec2 hw = 0.5 * max(fwidth(p), vec2(1e-5));
    vec2 cov = smoothstep(-hw, hw, p) * smoothstep(-hw, hw, 1.0 - p);
    float coverage = cov.x * cov.y;

    float a = topColor.a * alpha * coverage;
    vec3 blended = clamp(blend(baseColor.rgb, topColor.rgb, u.params.x), 0.0, 1.0);
    vec3 outRgb = mix(baseColor.rgb, blended, a);
    float outA = baseColor.a + a * (1.0 - baseColor.a);
    fragColor = vec4(outRgb, outA);
}
