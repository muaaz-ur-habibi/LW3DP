#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

char *LoadShaderFromFile(char *path);
GLuint CreateCompileShader(GLenum type, const GLchar *shaderSource);
GLuint ShaderProgramCreateAttachLink(GLuint *shaders, int nShaders);
void DeleteShaders(GLuint *shaders, int nShaders);
void UseShaderProgram(GLuint program);
