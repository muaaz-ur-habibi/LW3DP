#ifndef TEXTURES_H
#define TEXTURES_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include "Renderer.h"
#include "Uniform.h"

void TextureCreateTexture(GLuint *texture, GLuint shader_program, char *texture_uniform_name, unsigned int texture_id_amount, char *texture_filepath);
GLuint TextureCreateFromFile(char *path, int *w, int *h, int *nColChnls, unsigned char **bytes, GLenum type);
void TextureApplyParams(GLenum type, GLint minmag_param, GLint wrap_param);
void TextureApplyImage(GLenum type, GLint color_format, GLsizei width, GLsizei height, const void *pixels);
void TextureBindTexture(Model_blueprint m, GLenum tex_type);
void TextureBindNoTexture(Model_blueprint *m);

#endif