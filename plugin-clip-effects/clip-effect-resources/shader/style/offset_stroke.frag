/**
 *
 * PixelFlow | Copyright (C) 2017 Thomas Diewald - http://thomasdiewald.com
 *
 * A Processing/Java library for high performance GPU-Computing (GLSL).
 * MIT License: https://opensource.org/licenses/MIT
 *
 */


#version 150

out vec4 out_threshold;

uniform vec2 wh_rcp;
uniform vec4 colA = vec4(1.0);
uniform vec4 colB = vec4(0.0);
uniform float thickness = 0;
uniform float offset = 0;
uniform sampler2D tex_dtnn;

#ifdef GL_OES_standard_derivatives
#extension GL_OES_standard_derivatives : enable
#endif

float aastep(float threshold, float value) {
    float afwidth = length(vec2(dFdx(value), dFdy(value))) * 0.70710678118654757;
    return smoothstep(threshold-afwidth, threshold+afwidth, value);
}

void main(){
  ivec2 posi = ivec2(gl_FragCoord.xy);
   vec2 posn = gl_FragCoord.xy * wh_rcp;
  ivec2 dtnn = ivec2(texture(tex_dtnn, posn).xy * 32767.0);
  float dist = length(vec2(dtnn - posi));

  float mixval = 1.0 - (aastep(offset, dist) - aastep(thickness + offset, dist)); // dist > threshold ? 0.0 : 1.0

  out_threshold = mix(colB, colA, mixval);
}
