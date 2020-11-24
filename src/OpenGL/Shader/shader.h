#ifndef ENGINE_OPENGL_SHADER
#define ENGINE_OPENGL_SHADER

#include <glad/glad.h>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <initializer_list>
#include <exception>

namespace Engine {
    class OpenGLProgram;

    struct ShaderException : public std::exception
    {
        ShaderException(std::string message);
        const char* what() const throw();
    private:
        std::string m_message;
    };

    struct OpenGLShader{
        OpenGLShader() = delete;
        OpenGLShader(GLenum type, const char* source);
        OpenGLShader(const OpenGLShader& other);
    private:
        GLuint m_id{0};
        GLenum m_type{};
        std::string m_source{};
        void compileShader();
        friend class OpenGLProgram;
    };

    class OpenGLProgram{
    private:
        GLuint m_program{0};
        std::map<GLenum, OpenGLShader> m_shaders{};

        void linkProgram();
        void cleanupShaders();
        void rollback();
    public:
        OpenGLProgram(std::initializer_list<OpenGLShader> shaders);
        ~OpenGLProgram();
        void use();
        GLuint getBlockIndex(const char* blockName);
        GLuint getProgram();

        void updateProgram(std::vector<OpenGLShader> newShaders);
    };
}


#endif