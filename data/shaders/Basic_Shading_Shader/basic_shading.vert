#version 330 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in float vMatIndex;
layout (location = 3) in float vTransIndex;

struct TransformProperties {
    mat4 modelMatrix;
    mat4 normalMatrix;
};

const int maxTransforms = 20;
uniform Transforms{
    TransformProperties transforms[maxTransforms];
};

uniform Camera{
    mat4 viewMatrix;
    mat4 viewMatrixInverse;
    mat4 projectionMatrix;
};

flat out int matIndex;
out vec3 position;
out vec3 normal;
out vec3 cameraPosition;
flat out vec3 objPosition;
void main()
{
    matIndex = int(vMatIndex);
    position = (transforms[int(vTransIndex)].modelMatrix * vec4(vPosition, 1.0)).xyz;
    normal = normalize((transforms[int(vTransIndex)].normalMatrix * vec4(vNormal, 1.0)).xyz);
    cameraPosition = (viewMatrixInverse * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.0);
}