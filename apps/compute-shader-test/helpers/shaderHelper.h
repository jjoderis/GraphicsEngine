#ifndef RT_SHADER_HELPER_H
#define RT_SHADER_HELPER_H

void checkShader(unsigned int shader);

void checkProgram(unsigned int program);

unsigned int createComputeProgram(const char *source);

#endif