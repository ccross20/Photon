#version 330 core
in vec3 vertex;
in vec2 aTexCoord;
uniform mat4 mvMatrix;
uniform mat4 projMatrix;
out vec2 uv;

void main()
{
        gl_Position = projMatrix * mvMatrix * vec4(vertex, 1.0);
        uv = aTexCoord;
}
