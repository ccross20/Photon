#version 440

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform sampler2D sinkTex;

layout(std140, binding = 1) uniform Gather {
    vec4 background;   // rgb = canvas background colour
} g;

void main()
{
    // Out-of-canvas samples read the background, matching the CPU sampler's clip.
    if (vUV.x < 0.0 || vUV.x > 1.0 || vUV.y < 0.0 || vUV.y > 1.0) {
        fragColor = vec4(g.background.rgb, 1.0);
    } else {
        // Flatten straight-alpha over the canvas background, so masked/transparent
        // regions drive the fixtures with the background colour.
        vec4 c = texture(sinkTex, vUV);
        fragColor = vec4(c.rgb * c.a + g.background.rgb * (1.0 - c.a), 1.0);
    }
}
