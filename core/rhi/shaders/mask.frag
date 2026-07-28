#version 440

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Mask {
    vec4 params;   // x = channel (0 alpha,1 luminance,2 r,3 g,4 b), y = invert
} u;

layout(binding = 1) uniform sampler2D sourceTex;
layout(binding = 2) uniform sampler2D maskTex;

void main()
{
    vec4 src = texture(sourceTex, vUV);
    vec4 m = texture(maskTex, vUV);

    int channel = int(u.params.x + 0.5);
    float mv;
    if (channel == 0)      mv = m.a;
    else if (channel == 1) mv = dot(m.rgb, vec3(0.2126, 0.7152, 0.0722));
    else if (channel == 2) mv = m.r;
    else if (channel == 3) mv = m.g;
    else                   mv = m.b;

    if (u.params.y > 0.5)
        mv = 1.0 - mv;

    fragColor = vec4(src.rgb, src.a * mv);
}
