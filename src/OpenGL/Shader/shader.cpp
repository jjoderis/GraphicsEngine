#include "shader.h"

Engine::OpenGLShader::OpenGLShader(GLenum type, const char* source) : m_type{type}, m_source{source} {}

Engine::OpenGLShader::OpenGLShader(const OpenGLShader& other) {
    m_id = other.m_id;
    m_type = other.m_type;
    m_source = other.m_source;
}

bool Engine::OpenGLShader::compileShader() {
    m_id = glCreateShader(m_type);
    const char* source{m_source.c_str()};
    glShaderSource(m_id, 1, &source, NULL);
    glCompileShader(m_id);

    int success;
    char infoLog[512];
    glGetShaderiv(m_id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(m_id, 512, NULL, infoLog);
        char typeString[20];

        switch(m_type) {
            case GL_VERTEX_SHADER:
                strcpy(typeString, "VERTEX");
                break;
            case GL_FRAGMENT_SHADER:
                strcpy(typeString, "FRAGMENT");
                break;
            case GL_GEOMETRY_SHADER:
                strcpy(typeString, "GEOMETRY");
                break;
            case GL_TESS_EVALUATION_SHADER:
                strcpy(typeString, "TESS_EVALUATION");
                break;
            case GL_TESS_CONTROL_SHADER:
                strcpy(typeString, "TESS_CONTROL");
                break;
            case GL_COMPUTE_SHADER:
                strcpy(typeString, "COMPUTE");
                break;
            default:
                printf("Shader compilation with unknown shader type!\n");
                exit(EXIT_FAILURE);
        }


        fprintf(stderr, "ERROR::SHADER::%s::COMPILATION:FAILED: \n%s\n", typeString, infoLog);
        glDeleteShader(m_id);
        return false;
    }

    return true;
}

Engine::OpenGLProgram::OpenGLProgram() {
    // create very basic shader program
    OpenGLShader vertexShader{
        GL_VERTEX_SHADER, 
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in float aMatIndex;\n"
        "layout (location = 2) in float aTransIndex;\n"

        "struct MaterialProperties {\n"
        "   vec4 color;\n"
        "};\n\n"

        "const int maxMaterials = 20;\n"
        "uniform Materials{\n"
        "   MaterialProperties material[maxMaterials];\n"
        "};\n\n"

        "const int maxModelMatrices = 20;\n"
        "uniform Transforms{\n"
        "   mat4 model[maxModelMatrices];\n"
        "};\n\n"

        "uniform Camera{\n"
        "   mat4 viewMatrix;\n"
        "   mat4 viewMatrixInverse;\n"
        "   mat4 projectionMatrix;\n"
        "};\n\n"

        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "   color = material[int(aMatIndex)].color;\n"
        "   gl_Position = projectionMatrix * viewMatrix * model[int(aTransIndex)] * vec4(aPos, 1.0);\n"
        "}\0"
    };
    bool success = vertexShader.compileShader();

    if (!success) {
        printf("Failed to compile most basic shader!\n");
        exit(EXIT_FAILURE);
    }

    OpenGLShader fragmentShader{
        GL_FRAGMENT_SHADER,
        "#version 330 core\n"
        "in vec4 color;"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "    FragColor = color;\n"
        "}\n"
    };
    success = fragmentShader.compileShader();

    if (!success) {
        printf("Failed to compile most basic shader!\n");
        exit(EXIT_FAILURE);
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vertexShader.m_id);
    glAttachShader(m_program, fragmentShader.m_id);

    success = linkProgram();

    if (!success) {
        printf("Failed to link most basic program!\n");
        exit(EXIT_FAILURE);
    }

    m_shaders.push_back(vertexShader);
    m_shaders.push_back(fragmentShader);
}

Engine::OpenGLProgram::~OpenGLProgram() {
    for (Engine::OpenGLShader shader: m_shaders) {
        glDeleteShader(shader.m_id);
    }

    glDeleteProgram(m_program);
}

bool Engine::OpenGLProgram::linkProgram() {
    glLinkProgram(m_program);
    int success{0};
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_program, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING::FAILED\n %s\n", infoLog);
        return false;
    }

    return true;
}

void Engine::OpenGLProgram::use() {
    glUseProgram(m_program);
}

GLuint Engine::OpenGLProgram::getBlockIndex(const char* blockName) {
    return glGetUniformBlockIndex(m_program, blockName);
}

GLuint Engine::OpenGLProgram::getProgram() {
    return m_program;
}



