#version 330 core
attribute vec3 aPos;
attribute vec2 aUV;
attribute vec3 aOffset;

out vec3 FragPos;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos + aOffset, 1.0));
    TexCoord = aUV;

    gl_Position = projection * vec4(FragPos, 1.0);
}
