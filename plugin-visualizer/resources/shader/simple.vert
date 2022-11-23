#version 330 core
attribute vec3 vertex;
uniform mat4 view_matrix;

void main()
{
        gl_Position = view_matrix * vec4(vertex, 1.0);
}
