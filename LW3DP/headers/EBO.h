#include <stdio.h>
#include "VAO.h"

void EBOBindWithData(GLuint EBO, GLsizeiptr size, const void *data, GLenum usage);
void EBODraw(GLsizei count, GLenum type, GLuint VAO);
void EBODump(GLuint EBO, GLsizeiptr size, GLenum index_type);
