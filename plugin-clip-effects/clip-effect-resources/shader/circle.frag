#version 150
in vec2 uv;
out vec4 FragColor;
uniform vec2 ratio;
uniform float aspect;
uniform float falloff;
uniform vec4 fillColor = vec4(1.0);
uniform vec4 transparent = vec4(0.0);

float sdCircle( vec2 p, float r )
{
    return length(p) - r;
}

float aastep(float threshold, float value) {
    float afwidth = length(vec2(dFdx(value), dFdy(value))) * 0.70710678118654757;
    return smoothstep(threshold-afwidth, threshold+afwidth, value);
}

void main(void)
{

    vec2 p = 2.0*(uv-vec2(.5,.5));

    float d = sdCircle( p, ratio.x);


    if(falloff > 0.0)
        FragColor =  mix( transparent, fillColor, max(min(-d/falloff,1.0),0.0));
    else
        FragColor =  mix( transparent, fillColor, 1.0-aastep(0.0,d));
}
