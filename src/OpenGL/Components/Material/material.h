#ifndef ENGINE_OPENGL_COMPONENTS_MATERIAL
#define ENGINE_OPENGL_COMPONENTS_MATERIAL

#include "../../../Core/Math/math.h"
#include <tuple>
#include <vector>

namespace Engine
{
// contains information about which material properties exist and how to access them
// name, type, offset
using MaterialUniformData = std::tuple<std::string, unsigned int, int>;
// contains information about the size of the material information and the informations about material properties
// size, materialUniformData (vector)
using ShaderMaterialData = std::tuple<int, std::vector<MaterialUniformData>>;
class OpenGLMaterialComponent
{
private:
    ShaderMaterialData m_dataInfo{0, std::vector<MaterialUniformData>{}};
    std::vector<char> m_data;

public:
    OpenGLMaterialComponent() {}

    void setMaterialData(const ShaderMaterialData &materialData);
    ShaderMaterialData &getMaterialData();

    template <typename T>
    T *getProperty(int offset)
    {
        return (T *)(m_data.data() + offset);
    }

    std::vector<char> &getData();
};
} // namespace Engine

#endif