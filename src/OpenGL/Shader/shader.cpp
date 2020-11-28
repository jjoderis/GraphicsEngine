#include "shader.h"

Engine::ShaderException::ShaderException(std::string message) : m_message{message} {}

const char* Engine::ShaderException::what() const throw() {
    return m_message.c_str();
}

Engine::OpenGLShader::OpenGLShader(GLenum type, const char* source) : m_type{type}, m_source{source} {}

Engine::OpenGLShader::OpenGLShader(const OpenGLShader& other) {
    m_id = other.m_id;
    m_type = other.m_type;
    m_source = other.m_source;
}

void Engine::OpenGLShader::compileShader() {
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
        char extendedLog[640];
        sprintf(extendedLog, "ERROR::SHADER::%s::COMPILATION:FAILED: \n%s\n", typeString, infoLog);
        glDeleteShader(m_id);
        throw ShaderException{extendedLog};
    }
}

Engine::OpenGLProgram::OpenGLProgram(std::vector<OpenGLShader> shaders) {
    for (OpenGLShader shader: shaders) {
        try {
            shader.compileShader();
            m_shaders.emplace(shader.m_type, shader);
        } catch (ShaderException& err) {
            cleanupShaders();
            throw err;
        }
    }

    m_program = glCreateProgram();

    for (auto& entry: m_shaders) {
        glAttachShader(m_program, entry.second.m_id);
    }

    try{
        linkProgram();
    } catch (ShaderException& err) {
        cleanupShaders();
        throw err;
    }
}

void Engine::OpenGLProgram::cleanupShaders() {
    for (auto& entry: m_shaders) {
        glDeleteShader(entry.second.m_id);
    }
}

Engine::OpenGLProgram::~OpenGLProgram() {
    cleanupShaders();

    glDeleteProgram(m_program);
}

void Engine::OpenGLProgram::linkProgram() {
    glLinkProgram(m_program);
    int success{0};
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_program, 512, NULL, infoLog);
        char extendedLog[640];
        sprintf(extendedLog, "ERROR::SHADER::PROGRAM::LINKING::FAILED\n %s\n", infoLog);
        glDeleteProgram(m_program);
        throw ShaderException{extendedLog};
    }
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

void Engine::OpenGLProgram::rollback() {
    glDeleteProgram(m_program);

    // recreate a program that was like the old one
    m_program = glCreateProgram();

    for (auto& entry: m_shaders) {
        glAttachShader(m_program, entry.second.m_id);
    }

    glLinkProgram(m_program);
}

void Engine::OpenGLProgram::updateProgram(std::vector<OpenGLShader> newShaders) {
    int compiled{0};

    for (OpenGLShader& newShader: newShaders) {
        try {
            newShader.compileShader();
        } catch(ShaderException& err) {
            // rollback to old version and delete all newly compiled shaders
            for (int i{0}; i < compiled; ++i) {
                glDeleteShader(newShaders[i].m_id);
            }

            rollback();

            throw err;
        }

        // if there is a shader of the same type detach and attach the new shader
        if (m_shaders.find(newShader.m_type) != m_shaders.end()) {
            OpenGLShader& oldShader{ m_shaders.at(newShader.m_type) };

            glDetachShader(m_program, oldShader.m_id);
        }

        glAttachShader(m_program, newShader.m_id);

        ++compiled;
    }

    try {
        glLinkProgram(m_program);
    } catch (ShaderException& err) {
        // rollback to old version and delete all newly compiled shaders
        for (int i{0}; i < compiled; ++i) {
            glDeleteShader(newShaders[i].m_id);
        }

        rollback();

        throw err;
    }

    // new shaders are usable; delete old ones and replace with new ones
    for(OpenGLShader& newShader: newShaders) {
        if (m_shaders.find(newShader.m_type) != m_shaders.end()) {
            OpenGLShader& oldShader{ m_shaders.at(newShader.m_type) };
            glDeleteShader(oldShader.m_id);
            oldShader.m_id = newShader.m_id;
            oldShader.m_source = newShader.m_source;
        } else {
            m_shaders.emplace(newShader.m_type, newShader);
        }
    }
}

GLenum Engine::shaderPathToType(const fs::path& filePath) {
    fs::path extension{filePath.extension()};

    if (extension == ".vert") {
        return GL_VERTEX_SHADER;
    } else if (extension == ".frag") {
        return GL_FRAGMENT_SHADER;
    } else if (extension == ".geom") {
        return GL_GEOMETRY_SHADER;
    } else if (extension == ".teval") {
        return GL_TESS_EVALUATION_SHADER;
    } else if (extension == ".tcont") {
        return GL_TESS_CONTROL_SHADER;
    } else if (extension == ".comp") {
        return GL_COMPUTE_SHADER;
    } else {
        throw ShaderException{"Unknown file extension for shader loading."};
    }
}

std::string Engine::shaderTypeToExtension(GLenum type) {
    switch (type) {
        case GL_VERTEX_SHADER:
            return ".vert";
            break;
        case GL_FRAGMENT_SHADER:
            return ".frag";
            break;
        case GL_GEOMETRY_SHADER:
            return ".geom";
            break;
        case GL_TESS_EVALUATION_SHADER:
            return ".teval";
            break;
        case GL_TESS_CONTROL_SHADER:
            return ".tcont";
            break;
        case GL_COMPUTE_SHADER:
            return ".comp";
            break;
        default:
            throw ShaderException{"Unknown shader type for extension deduction."};
            break;
    }
}

Engine::OpenGLShader Engine::loadShader(const fs::path& filePath) {
    GLenum type{ shaderPathToType(filePath) };

    std::string extension{filePath.extension().string()};

    std::string shaderSource{Util::readTextFile(filePath.c_str())};

    return OpenGLShader{type, shaderSource.c_str()};
}

std::vector<Engine::OpenGLShader> Engine::loadShaders(const char* directoryPaths) {
    std::vector<OpenGLShader> shaders{};

    for(auto& filePath: Util::getFilePaths(directoryPaths)) {
        shaders.push_back(loadShader(filePath));
    }

    return shaders;
}

std::vector<Engine::OpenGLShader> Engine::OpenGLProgram::getShaders() {
    std::vector<OpenGLShader> shaders{};

    for (auto& entry: m_shaders) {
        shaders.push_back(OpenGLShader{entry.first, entry.second.m_source.c_str()});
    }

    return shaders;
}

void Engine::saveShader(const fs::path& path ,const OpenGLShader& shader) {
    fs::path filePath{path};
    bool savingAllowed{false};
    std::string expectedExtension{shaderTypeToExtension(shader.m_type)};

    // check if the path has a file extension
    if (path.has_extension()) {
        // check if the given extension is correct for the current shader type
        savingAllowed = expectedExtension == path.extension();
    } else {
        // add correct extension to path
        filePath += expectedExtension;
        savingAllowed = true;
    }

    if (savingAllowed) {
        Util::writeTextToFile(filePath.c_str(), shader.m_source);
    } else {
        throw ShaderException("Wrong file extension for a shader of the given type. Didn't write to file system!");
    }
}

void Engine::saveShaders(const fs::path& directoryPath, const std::vector<OpenGLShader>& shaders) {
    if (!fs::is_directory(directoryPath)) {
        throw ShaderException("Given path is not a directory. Didn't write to file system!");
    }

    std::string directoryName{directoryPath.stem().string()};

    for (const OpenGLShader& shader: shaders) {
        saveShader(fs::path{directoryPath / directoryName}, shader);
    }
}



