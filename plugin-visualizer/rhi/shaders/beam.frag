#version 440

layout(location = 0) in float vAlpha;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Frame {
    mat4 viewProj;
    vec4 lightDir;
} frame;

layout(std140, binding = 1) uniform Object {
    mat4 model;
    vec4 color;   // rgb = emitted color, a = beam gain
} object;

void main()
{
    // Premultiplied output for additive (ONE, ONE) blending.
    float a = object.color.a * vAlpha;
    fragColor = vec4(object.color.rgb * a, a);
}
