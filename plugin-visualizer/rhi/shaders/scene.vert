#version 440

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 vNormal;

layout(std140, binding = 0) uniform Frame {
    mat4 viewProj;
    vec4 lightDir;   // xyz = direction, w unused
} frame;

layout(std140, binding = 1) uniform Object {
    mat4 model;
    vec4 color;
} object;

void main()
{
    vNormal = mat3(object.model) * normal;
    gl_Position = frame.viewProj * object.model * vec4(position, 1.0);
}
