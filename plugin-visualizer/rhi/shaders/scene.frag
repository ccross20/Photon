#version 440

layout(location = 0) in vec3 vNormal;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform Frame {
    mat4 viewProj;
    vec4 lightDir;
    vec4 camPos;
    vec4 ambient;    // rgb = summed ambient lights, w = 1 if the scene has any
    vec4 dirCount;   // x = number of directional lights (0 = use the built-in key)
    vec4 dirLights[8]; // per light: (direction it shines along), (color * intensity)
} frame;

layout(std140, binding = 1) uniform Object {
    mat4 model;
    vec4 color;
} object;

void main()
{
    vec3 n = normalize(vNormal);
    float diffuse = max(dot(n, normalize(-frame.lightDir.xyz)), 0.0);
    // With ambient lights the room's ambient sets the level, plus a little
    // floor so fixtures and truss stay readable in a dark room. Without any,
    // the original fixed lighting.
    bool hasAmbient = frame.ambient.w > 0.5;
    vec3 ambient = hasAmbient ? frame.ambient.rgb + vec3(0.08) : vec3(0.3);

    // The scene's directional lights replace the built-in key light.
    vec3 key;
    int dirCount = int(frame.dirCount.x + 0.5);
    if (dirCount > 0) {
        key = vec3(0.0);
        for (int i = 0; i < dirCount; ++i)
            key += frame.dirLights[i * 2 + 1].rgb * max(dot(n, -frame.dirLights[i * 2].xyz), 0.0);
    } else {
        key = vec3(diffuse * (hasAmbient ? 0.4 : 0.7));
    }
    vec3 lit = object.color.rgb * (ambient + key);
    // object.color.a is an emissive factor: 1 = unlit, full-brightness (lens glow).
    float emissive = object.color.a;
    vec3 rgb = mix(lit, object.color.rgb, emissive);
    fragColor = vec4(rgb, 1.0);
}
