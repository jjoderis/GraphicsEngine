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
using ShaderMaterialData = std::pair<int, std::vector<MaterialUniformData>>;
class OpenGLMaterialComponent
{
public:
    OpenGLMaterialComponent() {}
    ~OpenGLMaterialComponent();

    void setMaterialData(const ShaderMaterialData &materialData);
    ShaderMaterialData &getMaterialData();
    void update();

    template <typename T>
    T *getProperty(int offset)
    {
        return (T *)(m_data.data() + offset);
    }

    std::vector<char> &getData();
    void bind();

private:
    ShaderMaterialData m_dataInfo{0, std::vector<MaterialUniformData>{}};
    std::vector<char> m_data;
    unsigned int m_UBO{0};
};
} // namespace Engine

#endif