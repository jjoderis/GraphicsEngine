#version 330 core
struct MaterialProperties {
    vec4 color;
};

const int maxMaterials = 20;
uniform Materials{
    MaterialProperties material[maxMaterials];
};

struct LightProperties {
    vec3 position;
};

const int maxLights = 20;
layout (std140) uniform Lights{
    int numLights;
    LightProperties lights[maxLights];
};

flat in int matIndex;
in vec3 position;
in vec3 normal;
in vec3 cameraPosition;

out vec4 FragColor;
void main()
{
    vec3 accColor = vec3(0, 0 ,0);
    vec3 viewDirection = normalize(cameraPosition - position);
    for (int i = 0; i < numLights; ++i){
        vec3 lightDirection = normalize(lights[i].position - position);
        float s = clamp(dot(lightDirection, normal), 0, 1);
        accColor = accColor + s * material[matIndex].color.xyz;
    }

    FragColor = vec4( accColor / numLights , 1.0);
}