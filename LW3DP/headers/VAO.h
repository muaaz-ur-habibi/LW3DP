#ifndef VAO_H
#define VAO_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "VBO.h"

typedef struct
{
    GLuint layout, size;
    GLenum type;
    GLboolean normalized;
    GLsizei stride;
    const void *ptr;
} VAOAttribute;

void VAOBindVertexArray(GLuint VAO);
void VAOAttribSetAndEnable(VAOAttribute attribute, GLuint VBO);
VAOAttribute *VAOCreateVAOAttributeArrays(int amount);

#endif