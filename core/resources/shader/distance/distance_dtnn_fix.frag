/**
 *
 * PixelFlow | Copyright (C) 2017 Thomas Diewald - http://thomasdiewald.com
 *
 * A Processing/Java library for high performance GPU-Computing (GLSL).
 * MIT License: https://opensource.org/licenses/MIT
 *
 */


 //
 // resouces
 //
 // Jumpflood Algorithm (JFA)
 //
 // Jump Flooding in GPU with Applications to Voronoi Diagram and Distance Transform
 // www.comp.nus.edu.sg/~tants/jfa/i3d06.pdf
 //



#version 330 core



#define POS_MAX 0x7FFF // == 32767 == ((1<<15) - 1)
#define LENGTH_SQ(dir) ((dir).x*(dir).x + (dir).y*(dir).y)




// --------------------------------------------------------------
// PASS 2 - update positions (nearest)
// --------------------------------------------------------------


#define TEXACCESS 0

#if (TEXACCESS == 0)
  // needs the position to be clamped
  #define getDTNN(tex, off) ivec2(texelFetch(tex, clamp(pos + off, ivec2(0), wh - 1), 0).xy * 32767.0)
#endif
#if (TEXACCESS == 1)
  // needs wrap-param GL_CLAMP_TO_EDGE
  #define getDTNN(tex, off) ivec2(texture(tex, (gl_FragCoord.xy + off) / wh).xy * 32767.0)
#endif

in vec2 uv;
out vec4 out_dtnn;

uniform sampler2D tex_dtnn;
uniform ivec3 jump;
uniform ivec2 wh;

ivec2 pos  = ivec2(gl_FragCoord.xy);
ivec2 dtnn = ivec2(32767);
int   dmin = LENGTH_SQ(dtnn);

void DTNN(const in ivec2 off){
  ivec2 dtnn_cur = getDTNN(tex_dtnn, off);
  ivec2 ddxy = dtnn_cur - pos;
  int dcur = LENGTH_SQ(ddxy);
  if(dcur < dmin){
    dmin = dcur;
    dtnn = dtnn_cur;
  }
}

void main(){

  dtnn = getDTNN(tex_dtnn, jump.yy);
  ivec2 ddxy = dtnn - pos;
  dmin = LENGTH_SQ(ddxy);

  DTNN(jump.xx); DTNN(jump.yx); DTNN(jump.zx);
  DTNN(jump.xy);                DTNN(jump.zy);
  DTNN(jump.xz); DTNN(jump.yz); DTNN(jump.zz);

  out_dtnn = vec4(vec2(dtnn) / 32767.0,1,1);

}

// void main(){
  // DTNN(jump.xx); DTNN(jump.yx); DTNN(jump.zx);
  // DTNN(jump.xy); DTNN(jump.yy); DTNN(jump.zy);
  // DTNN(jump.xz); DTNN(jump.yz); DTNN(jump.zz);

  // out_dtnn = dtnn;
// }

