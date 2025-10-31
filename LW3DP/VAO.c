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

void VAODraw(GLuint VAO, GLsizei count)
{
    VAOBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, count);
}

VAOAttribute *VAOCreateVAOAttributeArrays(int amount)
{
    return (VAOAttribute *)malloc(sizeof(VAOAttribute) * amount);
}

void VAODump(GLuint VAO, GLsizeiptr size)
{
    
    VAOBindVertexArray(VAO);
    
    printf("=== VAO STATE DUMP (VAO: %u) ===\n", VAO);
    
    // Check which vertex attributes are enabled
    for (GLuint i = 0; i < 16; i++) { // Check first 16 attributes
        GLint enabled;
        glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &enabled);
        if (enabled) {
            GLint size, type, normalized, stride;
            GLvoid *pointer;
            GLint buffer;
            
            glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_SIZE, &size);
            glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_TYPE, &type);
            glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED, &normalized);
            glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_STRIDE, &stride);
            glGetVertexAttribPointerv(i, GL_VERTEX_ATTRIB_ARRAY_POINTER, &pointer);
            glGetVertexAttribiv(i, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &buffer);
            
            printf("Attribute %d: size=%d, type=%d, normalized=%d, stride=%d, pointer=%p, buffer=%d\n",
                   i, size, type, normalized, stride, pointer, buffer);
        }
    }
    
    // Check which buffers are bound
    GLint array_buffer, element_buffer;
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &array_buffer);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &element_buffer);
    printf("Array Buffer Bound: %d, Element Array Buffer Bound: %d\n", 
           array_buffer, element_buffer);
    
    glBindVertexArray(0);
}