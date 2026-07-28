#version 440

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Tile {
    vec4 params;   // x,y = tile counts, z = mirror x, w = mirror y
} u;

layout(binding = 1) uniform sampler2D inputTex;

// Wrap a scaled coordinate back into [0,1]. Repeat = sawtooth (fract); mirror =
// triangle wave, which folds the texture so tiles meet seamlessly.
float wrap(float t, float mirror)
{
    if (mirror > 0.5)
        return 1.0 - abs(mod(t, 2.0) - 1.0);
    return fract(t);
}

void main()
{
    vec2 t = vUV * u.params.xy;
    vec2 uv = vec2(wrap(t.x, u.params.z), wrap(t.y, u.params.w));
    fragColor = texture(inputTex, uv);
}
