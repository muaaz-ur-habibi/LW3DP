#include "headers/EBO.h"

void EBOBindWithData(GLuint EBO, GLsizeiptr size, const void *data, GLenum usage)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    if (EBO != 0)
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
    }
}

void EBODraw(GLsizei count, GLenum type, GLuint VAO)
{
    VAOBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, count, type, (const void *)0);
}