#include "headers/Uniform.h"

void UniformSend4x4Matrix(GLuint shader_program, char *uniform_name, mat4 matrix)
{
    UseShaderProgram(shader_program);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, uniform_name), 1,
    GL_FALSE, (const float *)matrix);
}

void UniformSend4x4Matrices(GLuint shader_program, char *uniform_name, mat4 *matrices, int count)
{
    UseShaderProgram(shader_program);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, uniform_name), count,
    GL_FALSE, (const float *)matrices);
}

void UniformSendVec4(GLuint shader_program, char *uniform_name, vec4 vector)
{
    UseShaderProgram(shader_program);
    glUniform4fv(glGetUniformLocation(shader_program, uniform_name), 1, (float *)vector);
}

void UniformSendVec3(GLuint shader_program, char *uniform_name, vec3 vector)
{
    UseShaderProgram(shader_program);
    glUniform3fv(glGetUniformLocation(shader_program, uniform_name), 1, vector);
}

void UniformSendInt(GLuint shader_program, char *uniform_name, int integer)
{
    UseShaderProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, uniform_name), integer);
}