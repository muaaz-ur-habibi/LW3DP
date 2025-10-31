#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

void VBOBindWithData(GLuint VBO, GLsizeiptr size, const void *data, GLenum usage);
void VBOSimpleBind(GLuint VBO);
void VBODump(GLuint VBO, GLsizeiptr size);
