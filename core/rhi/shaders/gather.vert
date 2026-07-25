#version 440

// One point per pixel sample. clipPos places the point into a packed WxH target
// (computed on the CPU); uv is where it samples the canvas.
layout(location = 0) in vec2 clipPos;
layout(location = 1) in vec2 uv;

layout(location = 0) out vec2 vUV;

void main()
{
    vUV = uv;
    gl_Position = vec4(clipPos, 0.0, 1.0);
    gl_PointSize = 1.0;
}
