#include "shaderHelper.h"

#include <glad/glad.h>
#include <iostream>

void checkShader(unsigned int shader)
{
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    };
}

void checkProgram(unsigned int program)
{
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
}

unsigned int createComputeProgram(const char *source)
{
    auto shader{glCreateShader(GL_COMPUTE_SHADER)};
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    checkShader(shader);

    auto program{glCreateProgram()};
    glAttachShader(program, shader);
    glLinkProgram(program);

    checkProgram(program);

    return program;
}