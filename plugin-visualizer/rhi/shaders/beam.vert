#version 440

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 aux;   // aux.x = per-vertex alpha factor

layout(location = 0) out float vAlpha;

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
    vAlpha = aux.x;
    gl_Position = frame.viewProj * object.model * vec4(position, 1.0);
}
