#version 440

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Levels {
    vec4 inLevels;    // x = input black, y = input white, z = gamma
    vec4 outLevels;   // x = output black, y = output white
} u;

layout(binding = 1) uniform sampler2D inputTex;

// Photoshop-style levels: remap [inBlack,inWhite] -> [0,1] with a gamma (midtone)
// curve, then map that onto [outBlack,outWhite]. Applied to RGB; alpha untouched.
vec3 applyLevels(vec3 c)
{
    float inB = u.inLevels.x;
    float inW = u.inLevels.y;
    float gamma = max(u.inLevels.z, 1e-4);
    float outB = u.outLevels.x;
    float outW = u.outLevels.y;

    vec3 v = clamp((c - inB) / max(inW - inB, 1e-5), 0.0, 1.0);
    v = pow(v, vec3(1.0 / gamma));
    return outB + v * (outW - outB);
}

void main()
{
    vec4 c = texture(inputTex, vUV);
    fragColor = vec4(applyLevels(c.rgb), c.a);
}
