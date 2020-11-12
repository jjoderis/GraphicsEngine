#ifndef ENGINE_OPENGL_SHADER
#define ENGINE_OPENGL_SHADER

#include <glad/glad.h>
#include <string>
#include <vector>
#include <cstring>

namespace Engine {
    class OpenGLProgram;

    struct OpenGLShader{
        OpenGLShader() = delete;
        OpenGLShader(GLenum type, const char* source);
        OpenGLShader(const OpenGLShader& other);
    private:
        GLuint m_id{0};
        GLenum m_type{};
        std::string m_source{};
        bool compileShader();
        friend class OpenGLProgram;
    };

    class OpenGLProgram{
    private:
        GLuint m_program{0};
        std::vector<OpenGLShader> m_shaders{};

        bool linkProgram();
    public:
        OpenGLProgram();
        ~OpenGLProgram();
        void use();
        GLuint getBlockIndex(const char* blockName);
        GLuint getProgram();
    };
}


#endif