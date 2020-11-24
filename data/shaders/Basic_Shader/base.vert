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
void main()
{
    matIndex = int(vMatIndex);
    gl_Position = projectionMatrix * viewMatrix * transforms[int(vMatIndex)].modelMatrix * vec4(vPosition, 1.0);
}