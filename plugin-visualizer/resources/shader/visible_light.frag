#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D tex;

void main(void)
{
    FragColor = texture(tex, TexCoord) * vec4(1.0f,1.0f,1.0f,0.1f);
    //FragColor = vec4(1.0f,0.0f,0.5f,1.0f);
}
