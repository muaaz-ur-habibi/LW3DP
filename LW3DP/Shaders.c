#include "headers/Shaders.h"

char *LoadShaderFromFile(char *path)
{
    FILE *f = fopen(path, "rb");

    fseek(f, 0, SEEK_END);
    long fSize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buffer = (char *)malloc(fSize + 1);
    fread(buffer, 1, fSize, f);
    buffer[fSize] = '\0';

    fclose(f);

    return buffer;
}

GLuint CreateCompileShader(GLenum type, const GLchar *shaderSource)
{
    GLuint shader;
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);

    char log[512];
    glGetShaderInfoLog(shader, 512, NULL, log);
    printf("Shader Compile Log:\n%s\n", log);

    return shader;
}

GLuint ShaderProgramCreateAttachLink(GLuint *shaders, int nShaders)
{
    GLuint shaderProgram = glCreateProgram();
    
    for (size_t i = 0; i < nShaders; i++)
    {
        glAttachShader(shaderProgram, shaders[i]);
    }
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("Shader Link Error:\n%s\n", infoLog);
    }

    return shaderProgram;
}

void UseShaderProgram(GLuint program)
{
    glUseProgram(program);
}

void DeleteShaders(GLuint *shaders, int nShaders)
{
    for (size_t i = 0; i < nShaders; i++)
    {
        glDeleteShader(shaders[i]);
    }
}