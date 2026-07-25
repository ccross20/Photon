#version 440

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Xform {
    vec4 params;   // xy = translate (uv), z = rotation (radians), w = scale
} u;

layout(binding = 1) uniform sampler2D inputTex;

void main()
{
    // Sample the input under an inverse transform about the centre.
    vec2 uv = vUV - 0.5;
    float c = cos(u.params.z), s = sin(u.params.z);
    uv = mat2(c, -s, s, c) * uv;
    uv /= max(u.params.w, 1e-3);
    uv -= u.params.xy;
    uv += 0.5;
    fragColor = texture(inputTex, uv);
}
