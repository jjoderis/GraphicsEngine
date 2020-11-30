#version 330 core
struct MaterialProperties {
    vec4 color;
};

const int maxMaterials = 20;
uniform Materials{
    MaterialProperties material[maxMaterials];
};

struct DirectionalLightProperties {
    vec3 direction;
    vec3 color;
};

const int maxDirectionalLights = 20;
layout (std140) uniform DirectionalLights{
    int numDirectionalLights;
    DirectionalLightProperties directionalLights[maxDirectionalLights];
};

struct PointLightProperties {
    float intensity;
    vec3 position;
    vec3 color;
};

const int maxPointLights = 20;
layout (std140) uniform PointLights{
    int numPointLights;
    PointLightProperties pointLights[maxPointLights];
};

struct SpotLightProperties {
    float intensity;
    float cutoffAngle;
    float penumbraAngle;
    vec3 position;
    vec3 direction;
    vec3 color;
};

const int maxSpotLights = 20;
layout (std140) uniform SpotLights{
    int numSpotLights;
    SpotLightProperties spotLights[maxSpotLights];
};

flat in int matIndex;
in vec3 position;
in vec3 normal;

out vec4 FragColor;

vec3 calculatePointLightColors() {
    vec3 accColor = vec3(0.0, 0.0, 0.0);

    for (int i = 0; i < numPointLights; ++i){
        vec3 lightVec = pointLights[i].position - position;
        float lightDist = sqrt(dot(lightVec, lightVec));
        vec3 lightDirection = lightVec / lightDist;
        float s = clamp(dot(lightDirection, normal), 0, 1);
        // make light intensity go to zero at distance 1000
        float windowing = pow(max(0, 1 - pow(( lightDist / 1000.0), 4.0)), 2.0);
        // make light intensity fall off at a distance
        float attenuation = (16 * pointLights[i].intensity)/(pow(lightDist, 2.0) + 0.01);
        accColor = accColor + s * windowing * attenuation * pointLights[i].color * material[matIndex].color.xyz;
    }

    return accColor;
}

vec3 calculateSpotLightColors() {
    vec3 accColor = vec3(0.0, 0.0, 0.0);

    for (int i = 0; i < numSpotLights; ++i){
        vec3 lightVec = spotLights[i].position - position;
        float lightDist = sqrt(dot(lightVec, lightVec));
        vec3 lightDirection = lightVec / lightDist;
        float s = clamp(dot(lightDirection, normal), 0, 1);
        // make light intensity go to zero at distance 1000
        float windowing = pow(max(0, 1 - pow(( lightDist / 1000.0), 4.0)), 2.0);
        // make light intensity fall off at a distance
        float attenuation = (16 * spotLights[i].intensity)/(pow(lightDist, 2.0) + 0.01);

        float lightDot = dot(spotLights[i].direction, -lightDirection);
        float cosCutoff = cos(spotLights[i].cutoffAngle);
        float t = clamp((lightDot - cosCutoff)/(cos(spotLights[i].penumbraAngle) - cosCutoff), 0, 1);

        accColor = accColor + s * windowing * attenuation * pow(t, 2.0) * spotLights[i].color * material[matIndex].color.xyz;
    }

    return accColor;
}

vec3 calculateDirectionalLightColors() {
    vec3 accColor = vec3(0, 0 ,0);

    for (int i = 0; i < numDirectionalLights; ++i){
        float s = clamp(dot(-directionalLights[i].direction, normal), 0, 1);
        accColor = accColor + s * directionalLights[i].color * material[matIndex].color.xyz;
    }

    return accColor;
}


void main()
{
    vec3 accColor = calculateDirectionalLightColors() + calculatePointLightColors() + calculateSpotLightColors();
    int numLights = numDirectionalLights + numPointLights + numSpotLights;

    FragColor = vec4( accColor / numLights , 1.0);
}