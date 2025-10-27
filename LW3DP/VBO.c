#include "headers/VBO.h"


void VBOSimpleBind(GLuint VBO) { glBindBuffer(GL_ARRAY_BUFFER, VBO); }

void VBOBindWithData(GLuint VBO, GLsizeiptr size, const void *data, GLenum usage)
{
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}