#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "VAO.h"

void EBOBindWithData(GLuint EBO, GLsizeiptr size, const void *data, GLenum usage);
void EBODraw(GLsizei count, GLenum type, GLuint VAO);