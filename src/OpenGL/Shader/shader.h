#ifndef ENGINE_OPENGL_SHADER
#define ENGINE_OPENGL_SHADER

#include <filesystem>
#include <glad/glad.h>
#include <map>
#include <string>
#include <vector>

namespace Engine
{
class OpenGLProgram;

struct ShaderException : public std::exception
{
    ShaderException(std::string message);
    const char *what() const throw();

private:
    std::string m_message;
};

struct OpenGLShader
{
    OpenGLShader() = delete;
    OpenGLShader(GLenum type, const char *source);
    OpenGLShader(const OpenGLShader &other);
    GLenum m_type{};
    std::string m_source{};

private:
    GLuint m_id{0};
    void compileShader();
    friend class OpenGLProgram;
};

class OpenGLProgram
{
private:
    GLuint m_program{0};
    std::map<GLenum, OpenGLShader> m_shaders{};

    void linkProgram();
    void cleanupShaders();
    void rollback();

public:
    OpenGLProgram(std::vector<OpenGLShader> shaders);
    ~OpenGLProgram();
    void use();
    GLuint getBlockIndex(const char *blockName);
    GLuint getProgram();

    std::vector<OpenGLShader> getShaders();

    void updateProgram(std::vector<OpenGLShader> newShaders);
};

GLenum shaderPathToType(const std::filesystem::path &filePath);
std::string shaderTypeToExtension(GLenum type);

OpenGLShader loadShader(const std::filesystem::path &filePath);
std::vector<OpenGLShader> loadShaders(const char *directoryPaths);

void saveShader(const std::filesystem::path &path, const OpenGLShader &shader);
void saveShaders(const std::filesystem::path &directoryPath, const std::vector<OpenGLShader> &shaders);
} // namespace Engine

#endif