#ifndef RENDERER_H
#define RENDERER_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdlib.h>
#include <string.h>
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Shaders.h"
#include "Uniform.h"

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define IDX_BUFF 0
#define POS_VERTEXB 1
#define TEX_VERTEXB 2
#define NOR_VERTEXB 3
#define WVP_MAT_VERTEXB 4
#define WORLD_MAT_VERTEXB 5
#define NUM_BUFFERS 6


typedef struct
{
    char *name;
    char *texture_path;
    float Ka[3], Kd[3], Ks[3];
    float d_Tr;
    int illum;

    float texture_scale[3], texture_offset[3], texture_turbulence[3];
    float bump_multiplier;
} OBJ_Material;

typedef struct
{
    GLfloat vertex[9];        // 3 vertices × 3 components (x,y,z for each)
    GLfloat texture_coord[6]; // 3 vertices × 2 components (u,v for each)  
    GLfloat normal[9];        // 3 vertices × 3 components (nx,ny,nz for each)
} OBJ_face;

typedef struct
{
    GLfloat *vertices; // contains vertex coords, texture coords and normal coords in form x y z u v nx ny nz
    GLuint *indices;
    int vertex_count, index_count;
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
    int is_light;
} Model_blueprint;

Model_blueprint RendererCreateModelAOS(
    GLfloat *vertices, GLsizeiptr vertices_size,
    GLint *indices, GLsizeiptr indices_size,
    VAOAttribute *vao_attributes, int n_vao_attrs,
    char *vertex_shader_path, char *fragment_shader_path
);

Model_blueprint RendererCreateObjModel(OBJ_face *faces, int nfaces, char *vertex_shader_path, char *fragment_shader_path);
Model_blueprint *RendererCreateModel(Assimp_object asso, char *vertex_shader_path, char *fragment_shader_path);

void RendererCopyVec3ToModel(Model_blueprint *m, vec3 vector3);
void RendererRotateModel(Model_blueprint *m, float angles_in_rad[3]);

#endif