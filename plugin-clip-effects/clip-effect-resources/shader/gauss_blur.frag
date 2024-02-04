#version 330 core

#define PI (3.14159265)
#define TWO_PI (6.283185307)        //      2 * PI
#define TWO_PI_SQRT (2.506628274)   // sqrt(2 * PI)


in vec2 uv;
out vec4 glFragColor;

uniform sampler2D tex;
uniform vec2 wh_rcp;
uniform int radius;
uniform float sigma;
uniform vec2 dir;

void main(){

    vec3 coeff;
    coeff.x = 1.0 / (TWO_PI_SQRT * sigma);
    coeff.y = exp(-0.5 / (sigma * sigma));
    coeff.z = coeff.y * coeff.y;

    vec4 blur = vec4(0.0);
    float norm = 0.0;

    blur += texture(tex, gl_FragCoord.xy * wh_rcp) * coeff.x;
    norm += coeff.x;
    coeff.xy *= coeff.yz;

    for(int i = 1; i <= radius; i++){
        blur += texture(tex, (gl_FragCoord.xy + dir * i) * wh_rcp) * coeff.x;
        blur += texture(tex, (gl_FragCoord.xy - dir * i) * wh_rcp) * coeff.x;
        norm += coeff.x * 2.0;
        coeff.xy *= coeff.yz;
    }
    glFragColor = blur / norm;

}
