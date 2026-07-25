#version 440

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Fill {
    vec4 color1;   // solid colour / gradient start
    vec4 color2;   // gradient end
    vec4 params;   // x = mode (0 solid, 1 vertical, 2 horizontal)
} u;

void main()
{
    float mode = u.params.x;
    if (mode < 0.5)
        fragColor = u.color1;
    else if (mode < 1.5)
        fragColor = mix(u.color1, u.color2, clamp(vUV.y, 0.0, 1.0));
    else
        fragColor = mix(u.color1, u.color2, clamp(vUV.x, 0.0, 1.0));
}
