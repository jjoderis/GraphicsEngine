#include "shader.h"

Engine::OpenGLShaderComponent::OpenGLShaderComponent(std::vector<OpenGLShader> shaders) : m_program{shaders}
{
    setupUniforms();
}

void Engine::OpenGLShaderComponent::updateShaders(std::vector<OpenGLShader> &newShaders)
{
    try
    {
        m_program.updateProgram(newShaders);
        setupUniforms();
    }
    catch (ShaderException &err)
    {
        setupUniforms();
        throw err;
    }
}

std::vector<Engine::OpenGLShader> Engine::OpenGLShaderComponent::getShaders() { return m_program.getShaders(); }

void Engine::OpenGLShaderComponent::useShader() { m_program.use(); }

void Engine::OpenGLShaderComponent::setupUniforms()
{
    int blockIndex = m_program.getBlockIndex("Material");
    if (blockIndex >= 0)
    {
        glUniformBlockBinding(m_program.getProgram(), blockIndex, 0);
    }

    blockIndex = m_program.getBlockIndex("Transform");
    if (blockIndex >= 0)
    {
        glUniformBlockBinding(m_program.getProgram(), blockIndex, 1);
    }

    blockIndex = m_program.getBlockIndex("Camera");
    if (blockIndex >= 0)
    {
        glUniformBlockBinding(m_program.getProgram(), blockIndex, 2);
    }

    blockIndex = m_program.getBlockIndex("AmbientLights");
    if (blockIndex >= 0)
    {
        glUniformBlockBinding(m_program.getProgram(), blockIndex, 3);
    }

    blockIndex = m_program.getBlockIndex("DirectionalLights");
    if (blockIndex >= 0)
    {
        glUniformBlockBinding(m_program.getProgram(), blockIndex, 4);
    }

    blockIndex = m_program.getBlockIndex("PointLights");
    if (blockIndex >= 0)
    {
        glUniformBlockBinding(m_program.getProgram(), blockIndex, 5);
    }

    blockIndex = m_program.getBlockIndex("SpotLights");
    if (blockIndex >= 0)
    {
        glUniformBlockBinding(m_program.getProgram(), blockIndex, 6);
    }
}

int Engine::OpenGLShaderComponent::getLocation(const char* name) {
    return m_program.getLocation(name);
}

Engine::ShaderMaterialData Engine::OpenGLShaderComponent::getMaterialProperties()
{
    // get block index and check if there is a Materials block in use
    int blockIndex = m_program.getBlockIndex("Material");
    if (blockIndex < 0)
    {
        return ShaderMaterialData{0, std::vector<MaterialUniformData>{}};
    }

    unsigned int program{m_program.getProgram()};

    ShaderMaterialData materialData{0, std::vector<MaterialUniformData>{}};
    int &blockSize{std::get<0>(materialData)};
    std::vector<MaterialUniformData> &uniformData{std::get<1>(materialData)};

    // get the size of the block we will need
    glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    // get number of uniforms in the block
    int numUniforms;
    glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numUniforms);
    uniformData.reserve(numUniforms);

    // get indices of the active uniforms in the block
    std::vector<int> uniformIndices(numUniforms);
    glGetActiveUniformBlockiv(program, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, uniformIndices.data());

    // get each uniforms name
    char name[128];
    int type;
    int offset;
    int nameLength;
    for (unsigned int index : uniformIndices)
    {
        glGetActiveUniformName(program, index, 128 * sizeof(char), &nameLength, name);
        glGetActiveUniformsiv(program, 1, &index, GL_UNIFORM_TYPE, &type);
        glGetActiveUniformsiv(program, 1, &index, GL_UNIFORM_OFFSET, &offset);
        uniformData.emplace_back(MaterialUniformData{name, type, offset});
    }

    return materialData;
}