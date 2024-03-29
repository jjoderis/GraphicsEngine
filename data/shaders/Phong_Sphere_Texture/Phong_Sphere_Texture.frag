#version 330 core
uniform Material{
    vec4 diffuseColor;
    vec4 specularColor;
    float specularExponent;
};

struct AmbientLightProperties {
    vec3 color;
};

const int maxAmbientLights = 20;
layout (std140) uniform AmbientLights{
    int numAmbientLights;
    AmbientLightProperties ambientLights[maxAmbientLights];
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

in vec3 position;
in vec3 normal;
in vec3 cameraPosition;
in vec3 iPos;

uniform sampler2D texture1;

#define M_PI 3.1415926535897932384626433832795

vec3 calculateAmbientLightColors(vec3 diffuse) {
    vec3 accColor = vec3(0.0, 0.0, 0.0);

    for (int i = 0; i < numAmbientLights; ++i) {
        accColor = accColor + ambientLights[i].color * diffuse;
    }

    return accColor;
}

void getLightStats(vec3 lightPosition, float intensity, out float lightDist, out vec3 lightDirection, out float windowing, out float attenuation) {
vec3 lightVec = lightPosition - position;
lightDist = sqrt(dot(lightVec, lightVec));
lightDirection = lightVec / lightDist;
// make light intensity go to zero at distance 1000
windowing = pow(max(0, 1 - pow(( lightDist / 1000.0), 4.0)), 2.0);
    // make light intensity fall off at a distance
    attenuation = (8 * intensity)/(pow(lightDist, 2.0) + 0.01);
}

vec3 calculatePointLightColors(vec3 fNormal, vec3 viewDirection, vec3 diffuse, vec3 specular, float specularExp) {
    vec3 accColor = vec3(0, 0, 0);

    for (int i = 0; i < numPointLights; ++i){
        vec3 lightVec, lightDirection;
        float lightDist, windowing, attenuation;
        
        getLightStats(pointLights[i].position, pointLights[i].intensity, lightDist, lightDirection, windowing, attenuation);
        
        vec3 lightColor = windowing * attenuation * pointLights[i].color;
        
        float lightAngle = clamp(dot(lightDirection, fNormal), 0, 1);
        
        accColor += lightColor * lightAngle * diffuse;
        
        vec3 reflected = reflect(-lightDirection, fNormal);
        float s = clamp(dot(reflected, viewDirection), 0, 1);
        s = max(pow(s, specularExp), 0);      

        accColor += lightColor * specular * s;
    }

    return accColor;
}

vec3 calculateSpotLightColors(vec3 fNormal, vec3 viewDirection, vec3 diffuse, vec3 specular, float specularExp) {
    vec3 accColor = vec3(0, 0, 0);

    for (int i = 0; i < numSpotLights; ++i){
        vec3 lightVec, lightDirection;
        float lightDist, windowing, attenuation;
        
        getLightStats(spotLights[i].position, spotLights[i].intensity, lightDist, lightDirection, windowing, attenuation);
        
        float lightDot = dot(spotLights[i].direction, - lightDirection);
        float cosCutoff = cos(spotLights[i].cutoffAngle);
        float t = clamp((lightDot - cosCutoff)/(cos(spotLights[i].penumbraAngle) - cosCutoff), 0, 1);
        
        vec3 lightColor = windowing * attenuation * pow(t, 2.0) * spotLights[i].color;
        
        float lightAngle = clamp(dot(lightDirection, fNormal), 0, 1);
        
        accColor += lightColor * lightAngle * diffuse;

        vec3 reflected = reflect(-lightDirection, fNormal);
        float s = clamp(dot(reflected, viewDirection), 0, 1);
        s = max(pow(s, specularExp), 0);

        accColor += lightColor * specular * s;
    }

    return accColor;
}

vec3 calculateDirectionalLightColors(vec3 fNormal, vec3 viewDirection, vec3 diffuse, vec3 specular, float specularExp) {
    vec3 accColor = vec3(0, 0 ,0);

    for (int i = 0; i < numDirectionalLights; ++i){
        float lightAngle = clamp(dot(-directionalLights[i].direction, fNormal), 0, 1);
        vec3 lightColor = directionalLights[i].color;
        
        accColor += lightColor * diffuse * lightAngle;

        vec3 reflected = reflect(directionalLights[i].direction, fNormal);
        float s = clamp(dot(reflected, viewDirection), 0, 1);
        s = max(pow(s, specularExp), 0);
        
        accColor += lightColor * specular * s;
    }

    return accColor;
}

layout (location = 0) out vec4 FragColor;
layout (location = 1) out int index;
layout (location = 2) out vec3 worldPos;
uniform int fIndex;

void main()
{
    vec3 fNormal = normalize(normal);
    vec3 viewDirection = normalize(cameraPosition - position);

    float u = ((M_PI+atan(iPos.z, iPos.x))/(2*M_PI));
    float v = 1-((atan(sqrt(pow(iPos.x,2)+pow(-iPos.z,2)),iPos.y))/M_PI);
    vec2 uv = vec2(-u,v);

    vec3 diffuse = texture(texture1, uv).xyz;
    vec3 specular = specularColor.xyz;
    float specularExp = specularExponent;

    vec3 accColor = calculateAmbientLightColors(diffuse) + calculateDirectionalLightColors(fNormal, viewDirection, diffuse, specular, specularExp) + calculatePointLightColors(fNormal, viewDirection, diffuse, specular, specularExp) + calculateSpotLightColors(fNormal, viewDirection, diffuse, specular, specularExp);
    
    index = fIndex;
    worldPos = position;
    FragColor = vec4(accColor  , 1.0);
}
