#version 330 core
in vec3 aPosition;
in vec2 aTexCoord;
out vec2 uv;
void main()
{
    gl_Position = vec4(aPosition.x, aPosition.y, aPosition.z, 1.0);
    uv = aTexCoord;
}
