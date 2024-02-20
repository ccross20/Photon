#version 330 core


#define POS_MAX 0x7FFF // == 32767 == ((1<<15) - 1)
#define LENGTH_SQ(dir) ((dir).x*(dir).x + (dir).y*(dir).y)


in vec2 uv;
out vec4 out_pos;
//out ivec2 ColorOut;
uniform vec4 FG_mask;
uniform int  FG_invert = 0; // 1 to invert mask
uniform sampler2D	tex_mask;

ivec2 pos = ivec2(gl_FragCoord.xy);

int isFG(const in vec4 rgba){
  vec4 diff = rgba - FG_mask;
  float diff_sq = dot(diff, diff);
  return int(step(diff_sq, 0.0)) ^ FG_invert; // (rgba == FG_mask) ? 1 : 0
  //return (rgba == FG_mask) ? 1 : 0;
}

void main(){
  //vec4 rgba = texelFetch(tex_mask, pos, 0);
  vec4 rgba = texture(tex_mask, uv);

  out_pos = isFG(rgba) == 1 ? vec4(gl_FragCoord.x / 32767.0, gl_FragCoord.y / 32767.0,1,1) : vec4(1.0);
  //out_pos = isFG(rgba) == 1 ? vec4(0,0,0,1) : vec4(1.0);
  //out_pos = rgba;

}




