#version 440

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform sampler2D tex;

void main()
{
    // Flip Y so the canvas is y-down (origin top-left) like image/screen space,
    // which is what most users expect. The GPU pipeline renders y-up internally;
    // this is the display boundary. The DMX gather applies the matching flip.
    fragColor = texture(tex, vec2(vUV.x, 1.0 - vUV.y));
}
