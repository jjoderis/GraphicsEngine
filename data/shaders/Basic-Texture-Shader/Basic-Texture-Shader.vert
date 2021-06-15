#version 330 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;

uniform Transform{
    mat4 modelMatrix;
    mat4 normalMatrix;
};

uniform Camera{
    mat4 viewMatrix;
    mat4 viewMatrixInverse;
    mat4 projectionMatrix;
};

out vec3 position;
out vec3 normal;
out vec2 iTexCoord;
flat out vec3 objPosition;
void main()
{
    iTexCoord = vTexCoord;
    position = (modelMatrix * vec4(vPosition, 1.0)).xyz;
    normal = (normalMatrix * vec4(vNormal, 1.0)).xyz;
    gl_Position = projectionMatrix * viewMatrix * vec4(position, 1.0);
}
