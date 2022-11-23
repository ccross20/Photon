#version 330 core
attribute vec3 aPos;
attribute vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 inverseModel;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverseModel)) * aNormal;

    gl_Position = projection * vec4(FragPos, 1.0);
}
