#version 150
in vec2 uv;
out vec4 FragColor;
uniform sampler2D tex;
uniform float value1;
uniform float value2;


#define SQ3 1.7320508076

mat2 rot2d(float a) { return mat2(cos(a),-sin(a),sin(a),cos(a)); }

vec2 p6mmmap(vec2 uv, float repeats) {
    // clamp to a repeating box width 6x height 2x*sqrt(3)
    uv.x /= SQ3;
    uv = fract(uv * repeats - 0.5) - 0.5;
    uv.x *= SQ3;

    uv = abs(uv);

    vec2 st = uv;

    vec2 uv330 = rot2d(radians(330.)) * uv;
    if (uv330.x < 0.0){
        st.y = (st.y - 0.5) * -1.0;
        st.x *= SQ3;
        return st * 2.0;
    }
    else if (uv330.x > 0.5){
        st.x = (st.x - 0.5 * SQ3) * -1.0 * SQ3;
        return st * 2.0;
    }

    vec2 uv30 = rot2d(radians(30.)) * uv;
    if (uv30.y < 0.0 && uv30.x >= 0.5) st = vec2(1.0,1.0);
    else if (uv30.y >= 0.0 && uv30.x >= 0.5) st = vec2(-1.0,1.0);
    else if (uv30.y < 0.0 && uv30.x < 0.5) st = vec2(1.0,-1.0);
    else st = vec2(-1.0,-1.0);

    uv30.x = uv30.x - 0.5;
    uv = rot2d(radians(270.))* uv30;
    st = uv * st;
    st.x *= SQ3;
    return st * 2.0;
}

void main(void)
{
    FragColor = texture(tex, p6mmmap(uv,value1) * value2);
    //FragColor = texture(tex, p6mmmap(uv,2.) * 0.65);
}
