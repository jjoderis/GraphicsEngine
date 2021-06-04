#version 330 core
layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

uniform Transform{
    mat4 modelMatrix;
    mat4 normalMatrix;
};

uniform Camera{
    mat4 viewMatrix;
    mat4 viewMatrixInverse;
    mat4 projectionMatrix;
};

void main()
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vPosition, 1.0); 
}
