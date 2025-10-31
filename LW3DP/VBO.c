#include "headers/VBO.h"


void VBOSimpleBind(GLuint VBO) { glBindBuffer(GL_ARRAY_BUFFER, VBO); }

void VBOBindWithData(GLuint VBO, GLsizeiptr size, const void *data, GLenum usage)
{
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

void VBODump(GLuint VBO, GLsizeiptr size)
{
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // Map the buffer to read its contents
    GLfloat *data = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
    if (data) {
        printf("=== VBO DUMP (size: %ld bytes, %ld floats) ===\n", 
               size, size / sizeof(GLfloat));
        
        int num_floats = size / sizeof(GLfloat);
        for (int i = 0; i < num_floats; i++) {
            printf("VBO[%d] = %.3f\n", i, data[i]);
            
        }
        printf("\n");
        
        glUnmapBuffer(GL_ARRAY_BUFFER);
    } else {
        printf("Failed to map VBO\n");
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}