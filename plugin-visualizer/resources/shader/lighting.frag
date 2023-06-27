#version 420 core
out vec4 FragColor;

struct Material {
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct DirLight {
    vec3 direction;         // 16

    vec3 ambient;           // 16
    vec3 diffuse;           // 16
    vec3 specular;          // 16
};                          // = 64

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;          // 16
    vec3 direction;         // 12
    float cutOff;           // 4
    float outerCutOff;      // 4

    float constant;         // 4
    float linear;           // 4
    float quadratic;        // 4
    float cookieIndex;         // 4
    int dummy1;
    int dummy2;
    int dummy3;

    vec3 diffuse;           // 16
    mat4 lightMatrix;       // = 64
}; //132   144

#define NR_POINT_LIGHTS 4

in vec3 FragPos;
in vec3 Normal;

/*
layout (std140) uniform TestBlock
{
    float inner;
    float outer;
};
*/

layout (std140) uniform LightBlock
{
    SpotLight spotlights[10];       // 1440
    DirLight infiniteLights[2];     // 128
    int spotlightCount;             // 4
    int infiniteCount;              // 4
};                                  // = 1508


                                  // = 1508

uniform Material material;
uniform vec3 viewPos;
uniform int useTexture;
uniform sampler2D cookieTex;

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void main()
{
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // == =====================================================
    // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
    // For each phase, a calculate function is defined that calculates the corresponding color
    // per lamp. In the main() function we take all the calculated colors and sum them up for
    // this fragment's final color.
    // == =====================================================
    // phase 1: directional lighting
    vec3 result;

    for(int i = 0; i < infiniteCount; ++i)
    {
        result += CalcDirLight(infiniteLights[i], norm, viewDir);
    }

    // phase 2: point lights
    //result += CalcPointLight(pointLight, norm, FragPos, viewDir);
    // phase 3: spot light


    for(int i = 0; i < spotlightCount; ++i)
    {
        result += CalcSpotLight(spotlights[i], norm, FragPos, viewDir);
    }

    //result += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    FragColor = vec4(result, 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // combine results
    vec3 ambient = light.ambient * material.diffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    //return (ambient + diffuse + specular);
    return ambient + diffuse + specular;
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient * material.diffuse;
    vec3 diffuse = light.diffuse * diff * material.diffuse;
    vec3 specular = light.specular * spec * material.specular;
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 lightPos = vec3(light.lightMatrix * vec4(fragPos, 1.0));
    // combine results

    vec3 diffuse = light.diffuse * material.diffuse;

    if(light.cookieIndex > 0)
        diffuse = (light.diffuse * texture(cookieTex, (lightPos.xy/(distance*4.0) ) + vec2(0.5, 0.5)).r) * material.diffuse;

    //vec3 diffuse = (light.diffuse * texture(cookieTex, (lightPos.xy/(60.f) ) + vec2(0.5, 0.5)).r) * material.diffuse;

    //diffuse = light.diffuse * material.diffuse;
    //vec3 diffuse = light.diffuse * diff * material.diffuse;
    diffuse *= attenuation * intensity;
    return diffuse;
}
