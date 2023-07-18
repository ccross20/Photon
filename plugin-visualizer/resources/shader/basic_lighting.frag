#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 viewPos;


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
    int cookieIndex;         // 4
    int dummy1;
    int dummy2;
    int dummy3;

    vec3 diffuse;           // 16
    mat4 lightMatrix;       // = 64
}; //132   144

layout (std140) uniform LightBlock
{
    SpotLight spotlights[40];       // 1440
    DirLight infiniteLights[2];     // 128
    int spotlightCount;             // 4
    int infiniteCount;              // 4
};                                  // = 1508

uniform Material material;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result;

    for(int i = 0; i < infiniteCount; ++i)
    {
        result += CalcDirLight(infiniteLights[i], norm, viewDir);
    }

    FragColor = vec4(result, 1.0);
}

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
