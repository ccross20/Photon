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
uniform float spacing = 10.0;
uniform float offset = 0.0;
uniform int count = 1;
uniform sampler2D tex_dtnn;
//uniform isampler2D tex_dtnn;

#ifdef GL_OES_standard_derivatives
#extension GL_OES_standard_derivatives : enable
#endif

float aastep(float threshold, float value) {
  //#ifdef GL_OES_standard_derivatives
    float afwidth = length(vec2(dFdx(value), dFdy(value))) * 0.70710678118654757;
    return smoothstep(threshold-afwidth, threshold+afwidth, value);
    /*
  #else
    return step(threshold, value);
  #endif
  */
}

void main(){
  ivec2 posi = ivec2(gl_FragCoord.xy);
   vec2 posn = gl_FragCoord.xy * wh_rcp;
  //ivec2 dtnn = texture(tex_dtnn, posn).xy;

   ivec2 dtnn = ivec2(texture(tex_dtnn, posn).xy * 32767.0);


  float dist = length(vec2(dtnn - posi))- offset;
  //float mixval = aastep(thickness, dist); // dist > threshold ? 0.0 : 1.0

  float doubleThickness = thickness + spacing;
  float clampedDoubleDist = floor(dist / doubleThickness) * doubleThickness;
  float clampedDist = floor(dist / thickness) * thickness;
  float mixval = 1.0 - (aastep(clampedDoubleDist, dist) - aastep(thickness + clampedDoubleDist, dist));

  mixval = mix(mix(0.0,1.0,step(.001, offset)), mixval, aastep(1, dist));


  float countmix = step(doubleThickness * count, dist);


  out_threshold = mix(mix(colB, colA, mixval), vec4(0), countmix);
}
