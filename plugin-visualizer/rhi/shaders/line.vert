#version 440

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 vColor;

layout(std140, binding = 0) uniform Frame {
    mat4 viewProj;
    vec4 lightDir;
} frame;

void main()
{
    vColor = color;
    gl_Position = frame.viewProj * vec4(position, 1.0);
}
