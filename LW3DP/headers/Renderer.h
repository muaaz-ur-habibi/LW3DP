#ifndef RENDERER_H
#define RENDERER_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdlib.h>
#include <string.h>
#include "VAO.h"
#include "EBO.h"
#include "Uniform.h"

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

typedef struct
{
    int has_texture;
    unsigned int texture_id;
    char *texture_path;
} Assimp_texture;

typedef struct
{
    GLfloat *vertices; // contains vertex coords, texture coords and normal coords in form x y z u v nx ny nz
    GLuint *indices;
    int vertex_count, index_count;
    int parent_mesh;

    Assimp_texture texture;

    const char *mesh_name;
} Assimp_mesh;

typedef struct
{
    Assimp_mesh *meshes;
    int n_meshes;
} Assimp_object;

// AOS version
typedef struct
{
    GLfloat *vertices;
    GLuint *indices;
    int vertices_count, indices_count;
    GLsizeiptr vertices_size, indices_size;
    GLuint shader_program, VAO, VBO, EBO, texture;
    mat4 model;
    vec3 position;
    vec4 color;

    const char *model_name;
    int model_id, parent_id;
} Model_blueprint;

typedef struct
{
    vec4 combined_light_color;

} Light_models;

Model_blueprint RendererCreateModelAOS(
    GLfloat *vertices, GLsizeiptr vertices_size,
    GLint *indices, GLsizeiptr indices_size,
    VAOAttribute *vao_attributes, int n_vao_attrs,
    char *vertex_shader_path, char *fragment_shader_path
);

Model_blueprint *RendererCreateModel(Assimp_object asso, char *vertex_shader_path, char *fragment_shader_path);
Model_blueprint RendererCreateLight(char *light_vertex_shader, char *light_fragment_shader, vec4 light_color);

void RendererCopyVec3ToModel(Model_blueprint *m, vec3 vector3);
void RendererRotateModel(Model_blueprint *m, float angles_in_rad[3]);

#endif