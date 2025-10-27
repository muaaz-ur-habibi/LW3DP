#include "headers/Textures.h"

void TextureCreateTexture(GLuint *texture, GLuint shader_program, char *texture_uniform_name,
    unsigned int texture_id_amount, char *texture_filepath)
{
    int wImg, hImg, nColChannelsl;
    unsigned char *Tbytes = NULL;
    
    *texture = TextureCreateFromFile(texture_filepath, &wImg, &hImg, &nColChannelsl, &Tbytes, GL_TEXTURE_2D);
    
    if (Tbytes == NULL) { 
        printf("Error in texture image\n"); 
        return;
    }
    
    glActiveTexture(GL_TEXTURE0);
    
    glBindTexture(GL_TEXTURE_2D, *texture);
    TextureApplyParams(GL_TEXTURE_2D, GL_NEAREST, GL_REPEAT);
    
    GLenum actual_format = (nColChannelsl == 4) ? GL_RGBA : GL_RGB;
    TextureApplyImage(GL_TEXTURE_2D, actual_format, wImg, hImg, Tbytes);

    UniformSendInt(shader_program, texture_uniform_name, texture_id_amount);
    
    stbi_image_free(Tbytes);
}

GLuint TextureCreateFromFile(char *path, int *w, int *h, int *nColChnls, unsigned char **bytes, GLenum type)
{
    stbi_set_flip_vertically_on_load(GL_TRUE);
    *bytes = stbi_load(path, w, h, nColChnls, 0);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(type, texture);

    return texture;
}

void TextureApplyParams(GLenum type, GLint minmag_param, GLint wrap_param)
{
    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, minmag_param);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, minmag_param);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, wrap_param);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, wrap_param);
    
}

void TextureApplyImage(GLenum type, GLint color_format, GLsizei width, GLsizei height, const void *pixels)
{
    glTexImage2D(type, 0, color_format, width, height, 0, color_format, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(type);
}

void TextureBindTexture(Model_blueprint m, GLenum tex_type)
{
    UseShaderProgram(m.shader_program);
    VAOBindVertexArray(m.VAO);
    
    GLuint use_texture_loc = glGetUniformLocation(m.shader_program, "use_texture");
    
    if (m.texture != 0) {
        // We have a valid texture
        glBindTexture(tex_type, m.texture);
        UniformSendInt(m.shader_program, "use_texture", 1);
    } else {
        // No texture available
        UniformSendInt(m.shader_program, "use_texture", 0);
    }
}

void TextureBindNoTexture(Model_blueprint *m)
{
    GLuint use_texture_loc = glGetUniformLocation(m->shader_program, "use_texture");
    m->texture = 0;
    UniformSendInt(m->shader_program, "use_texture", 0);
}