#version 330
#extension GL_OES_standard_derivatives : enable

in vec3 worldPosition;
out vec4 outColor;
 void main(void)
 {
     // Pick a coordinate to visualize in a grid
       vec2 coord = worldPosition.xz*.01;

       // Compute anti-aliased world-space grid lines
       vec2 grid = abs(fract(coord - 0.5) - 0.5) / fwidth(coord);
       float line = min(grid.x, grid.y);

       // Just visualize the grid lines directly
       float color = 1.0 - min(line, 1.0);

       // Apply gamma correction
       color = pow(color, 1.0 / 2.2);
       outColor = mix(vec4(.5,0.0,0.0,0.0),vec4(.2,.2,.2, 1.0), color);
 }
