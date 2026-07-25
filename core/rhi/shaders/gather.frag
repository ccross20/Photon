#version 440

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform sampler2D sinkTex;

void main()
{
    // Out-of-canvas samples read black, matching the CPU sampler's behaviour.
    if (vUV.x < 0.0 || vUV.x > 1.0 || vUV.y < 0.0 || vUV.y > 1.0)
        fragColor = vec4(0.0);
    else
        fragColor = texture(sinkTex, vUV);
}
