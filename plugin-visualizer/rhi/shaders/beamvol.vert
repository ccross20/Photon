#version 440

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 aux;   // unused in volumetric mode

layout(location = 0) out vec3 vWorld;

layout(std140, binding = 0) uniform Frame {
    mat4 viewProj;
    vec4 lightDir;
    vec4 camPos;     // xyz = camera world position
} frame;

// Must match the Beam block in beamvol.frag exactly (GL links them as one).
layout(std140, binding = 1) uniform Beam {
    mat4 model;
    vec4 color;      // rgb = emitted color, a = gain
    vec4 apex;       // xyz = cone apex (world)
    vec4 axisCos;    // xyz = beam axis (world, unit), w = cos(half angle)
    vec4 params;     // x = length, y = gobo index, z = gobo rotation, w = color split
    vec4 color2;     // rgb = second color-wheel color (split)
    vec4 fadePlane;  // xyz = plane normal (toward apex), w = offset; zero = no fade
} beam;

void main()
{
    vec4 wp = beam.model * vec4(position, 1.0);
    vWorld = wp.xyz;
    gl_Position = frame.viewProj * wp;
}
