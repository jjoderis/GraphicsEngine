#version 330 core
uniform Material{
    vec4 color;
};

in vec3 position;

uniform int fIndex;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out int index;
layout (location = 2) out vec3 worldPos;

void main()
{
    FragColor = color;
    index = fIndex;
    worldPos = position;
}
