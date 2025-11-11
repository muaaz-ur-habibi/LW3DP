#include "headers/VBO.h"


void VBOSimpleBind(GLuint VBO) { glBindBuffer(GL_ARRAY_BUFFER, VBO); }

void VBOBindWithData(GLuint VBO, GLsizeiptr size, const void *data, GLenum usage)
{
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

void VBODump(GLuint VBO, GLsizeiptr size, int dump_file)
{
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // Map the buffer to read its contents
    GLfloat *data = (GLfloat*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
    if (data) {
        printf("=== VBO DUMP (size: %ld bytes, %ld floats) ===\n", 
               size, size / sizeof(GLfloat));
        
        int num_floats = size / sizeof(GLfloat);
        FILE *f = fopen("VBOdump.txt", "w");
        for (int i = 0; i < num_floats; i++) {
            if (dump_file)
            {
                char to_store[128];
                fprintf(f, "VBO[%d] = %.2f\n", i, data[i]);
            }
            else {
                printf("VBO[%d] = %.3f\n", i, data[i]);
            }
        }
        fclose(f);
        printf("\n");
        
        glUnmapBuffer(GL_ARRAY_BUFFER);
    } else {
        printf("Failed to map VBO\n");
    }
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}