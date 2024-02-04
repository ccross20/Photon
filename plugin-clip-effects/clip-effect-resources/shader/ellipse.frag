#version 150
in vec2 uv;
out vec4 FragColor;
uniform vec2 ratio;
uniform float aspect;
uniform float falloff;
uniform vec4 fillColor = vec4(1.0);
uniform vec4 transparent = vec4(0.0);

float sdEllipse(vec2 p, vec2 ab )
{

    p = abs(p); if( p.x > p.y ) {p=p.yx;ab=ab.yx;}


    float l = ab.y*ab.y - ab.x*ab.x;
    float m = ab.x*p.x/l;      float m2 = m*m;
    float n = ab.y*p.y/l;      float n2 = n*n;
    float c = (m2+n2-1.0)/3.0; float c3 = c*c*c;
    float q = c3 + m2*n2*2.0;
    float d = c3 + m2*n2;
    float g = m + m*n2;
    float co;
    if( d<0.0 )
    {
        float h = acos(min(q/c3, 1.0))/3.0;
        float s = cos(h);
        float t = sin(h)*sqrt(3.0);
        float rx = sqrt( -c*(s + t + 2.0) + m2 );
        float ry = sqrt( -c*(s - t + 2.0) + m2 );
        co = (ry+sign(l)*rx+abs(g)/(rx*ry)- m)/2.0;
    }
    else
    {
        float h = 2.0*m*n*sqrt(d);
        float s = sign(q+h)*pow(abs(q+h), 1.0/3.0);
        float u = sign(q-h)*pow(abs(q-h), 1.0/3.0);
        float rx = -s - u - c*4.0 + 2.0*m2;
        float ry = (s - u)*sqrt(3.0);
        float rm = sqrt(rx*rx + ry*ry);
        //co = min((h + 2.0 * g / rm - m) * 0.5, 1.0);
        co = (ry/sqrt(rm-rx)+2.0*g/rm-m)/2.0;
    }
    vec2 r = ab * vec2(co, sqrt(max(1.0-co*co, .000001 )));
    return length(r-p) * sign(p.y-r.y);
}

float sdEllipsoidApproximated(vec2 p, vec2 r )
{
    float k0 = length(p/r);
    float k1 = length(p/(r*r));
    return k0*(k0-1.0)/k1;
}

float aastep(float threshold, float value) {
    float afwidth = length(vec2(dFdx(value), dFdy(value))) * 0.70710678118654757;
    return smoothstep(threshold-afwidth, threshold+afwidth, value);
}


void main(void)
{

    vec2 p = 2.0*(uv-vec2(.5,.5));

    float d = sdEllipse( p, ratio );


    if(falloff > 0.0)
        FragColor =  mix( transparent, fillColor, max(min(-d/falloff,1.0),0.0));
    else
        FragColor =  mix( transparent, fillColor, 1.0-aastep(0.0,d) );
}
