#include <cglm/cglm.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Shaders.h"

void UniformSend4x4Matrix(GLuint shader_program, char *uniform_name, mat4 matrix);
void UniformSendInt(GLuint shader_program, char *uniform_name, int integer);
void UniformSendVec4(GLuint shader_program, char *uniform_name, vec4 vector);
void UniformSendVec3(GLuint shader_program, char *uniform_name, vec3 vector);
