#version 330 core
uniform Material{
    vec4 color;
};

out vec4 FragColor;
void main()
{
    FragColor = color;
}
