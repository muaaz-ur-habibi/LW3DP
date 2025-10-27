#include "headers/VAO.h"

void VAOBindVertexArray(GLuint VAO)
{
    glBindVertexArray(VAO);
}

void VAOAttribSetAndEnable(VAOAttribute attribute, GLuint VBO)
{
    VBOSimpleBind(VBO);
    glVertexAttribPointer(attribute.layout, attribute.size, attribute.type,
        attribute.normalized, attribute.stride, attribute.ptr);
    glEnableVertexAttribArray(attribute.layout);
    VBOSimpleBind(0);
}

VAOAttribute *VAOCreateVAOAttributeArrays(int amount)
{
    return (VAOAttribute *)malloc(sizeof(VAOAttribute) * amount);
}