#version 330 core
struct MaterialProperties {
    vec4 color;
};

const int maxMaterials = 20;
uniform Materials{
    MaterialProperties material[maxMaterials];
};


flat in int matIndex;
out vec4 FragColor;
void main()
{
    FragColor = material[matIndex].color;
}