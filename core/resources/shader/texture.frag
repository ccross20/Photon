#version 330 core
uniform sampler2D tex;
in vec2 uv;
out vec4 FragColor;

void main()
{
FragColor = texture(tex, uv);
//FragColor = vec4(1.f,1.f,0.f,1.f);
}
