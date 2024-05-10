#version 330 core
in vec2 uv;
out vec4 FragColor;

uniform sampler2D inputA;
uniform sampler2D inputB;

void main(void)
{
    FragColor = texture(inputA, uv) * texture(inputB, uv).a;
}
