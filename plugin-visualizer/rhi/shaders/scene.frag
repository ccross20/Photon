#version 440

layout(location = 0) in vec3 vNormal;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Frame {
    mat4 viewProj;
    vec4 lightDir;
} frame;

layout(std140, binding = 1) uniform Object {
    mat4 model;
    vec4 color;
} object;

void main()
{
    vec3 n = normalize(vNormal);
    float diffuse = max(dot(n, normalize(-frame.lightDir.xyz)), 0.0);
    const float ambient = 0.3;
    fragColor = vec4(object.color.rgb * (ambient + diffuse * 0.7), object.color.a);
}
