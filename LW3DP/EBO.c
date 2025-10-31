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

void EBODump(GLuint EBO, GLsizeiptr size, GLenum index_type)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    
    if (index_type == GL_UNSIGNED_INT) {
        GLuint *data = (GLuint*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY);
        if (data) {
            int num_indices = size / sizeof(GLuint);
            printf("=== EBO DUMP (%d indices) ===\n", num_indices);
            
            for (int i = 0; i < num_indices; i++) {
                printf("EBO[%d] = %u", i, data[i]);
                if ((i + 1) % 3 == 0) printf("  <- Triangle %d\n", (i + 1) / 3 - 1);
                else printf(" | ");
            }
            printf("\n");
            
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        }
    }
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}