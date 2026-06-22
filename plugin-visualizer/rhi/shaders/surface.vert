#version 440

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 vWorld;
layout(location = 1) out vec3 vNormal;

layout(std140, binding = 0) uniform Frame {
    mat4 viewProj;
    vec4 lightDir;
    vec4 camPos;
} frame;

layout(std140, binding = 1) uniform Object {
    mat4 model;
    vec4 color;
} object;

void main()
{
    vec4 wp = object.model * vec4(position, 1.0);
    vWorld = wp.xyz;
    vNormal = mat3(object.model) * normal;
    gl_Position = frame.viewProj * wp;
}
