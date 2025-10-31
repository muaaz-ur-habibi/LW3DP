#include "headers/Renderer.h"

Model_blueprint RendererCreateModelAOS(
    GLfloat *vertices, GLsizeiptr vertices_size,
    GLint *indices, GLsizeiptr indices_size,
    VAOAttribute *vao_attributes, int n_vao_attrs,
    char *vertex_shader_path, char *fragment_shader_path
)
{
    Model_blueprint model;
    model.vertices_size = vertices_size;
    model.indices_size = indices_size;

    model.vertices = (GLfloat *)malloc(model.vertices_size);
    model.indices = (GLint *)malloc(model.indices_size);

    if (model.vertices != NULL && model.indices != NULL)
    {
        memcpy(model.vertices, vertices, model.vertices_size);
        memcpy(model.indices, indices, model.indices_size);
    } else {printf("Error\n");}

    if (model.indices == NULL || model.vertices == NULL)
    {
        printf("Error\n");
    }

    char *vertex_shader = LoadShaderFromFile(vertex_shader_path);
    char *fragment_shader = LoadShaderFromFile(fragment_shader_path);

    GLuint vertex_shader_addr = CreateCompileShader(GL_VERTEX_SHADER, vertex_shader);
    GLuint frag_shader_addr = CreateCompileShader(GL_FRAGMENT_SHADER, fragment_shader);

    GLuint shaders[2] = {vertex_shader_addr, frag_shader_addr};
    GLuint shader_program = ShaderProgramCreateAttachLink(shaders, 2);
    DeleteShaders(shaders, 2);
    model.shader_program = shader_program;

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    VAOBindVertexArray(VAO);
    VBOBindWithData(VBO, model.vertices_size, model.vertices, GL_STATIC_DRAW);
    EBOBindWithData(EBO, model.indices_size, model.indices, GL_STATIC_DRAW);

    for (size_t i = 0; i < n_vao_attrs; i++)
    {
        VAOAttribSetAndEnable(vao_attributes[i], VBO);
    }

    model.VAO = VAO; model.VBO = VBO; model.EBO = EBO;

    VBOBindWithData(0, 0, (void *)0, 0);
    VAOBindVertexArray(0);
    EBOBindWithData(0, 0, (void *)0, 0);

    glm_mat4_identity(model.model);

    return model;
}

// creates the data for the vbo and ebo
Model_blueprint RendererCreateObjModel(OBJ_face *faces, int nfaces, char *vertex_shader_path, char *fragment_shader_path)
{
    Model_blueprint model;
    GLfloat *vertices_data = malloc( sizeof(GLfloat) * 24 * nfaces );
    int v_idx = 0;

    for (size_t i = 0; i < nfaces; i++) // for each face
    {
        for (size_t j = 0; j < 3; j++) // for each vertex
        {
            vertices_data[v_idx++] = faces[i].vertex[j * 3];
            vertices_data[v_idx++] = faces[i].vertex[(j * 3) + 1];
            vertices_data[v_idx++] = faces[i].vertex[(j * 3) + 2];

            vertices_data[v_idx++] = faces[i].texture_coord[j * 2];
            vertices_data[v_idx++] = faces[i].texture_coord[j * 2 + 1];

            vertices_data[v_idx++] = faces[i].normal[j * 3];
            vertices_data[v_idx++] = faces[i].normal[j * 3 + 1];
            vertices_data[v_idx++] = faces[i].normal[j * 3 + 2];
        }
    }
    
    GLint *indices_data = malloc( sizeof(GLint) * (v_idx/8) );
    int indices_written = 0;

    for (size_t i = 0; i < v_idx/8; i++)
    {
        indices_data[i] = i; indices_written++;
    }
    
    VAOAttribute *attribs = VAOCreateVAOAttributeArrays(3);
    attribs[0] = (VAOAttribute){0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)(0 * sizeof(float))};
    attribs[1] = (VAOAttribute){1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)(3 * sizeof(float))};
    attribs[2] = (VAOAttribute){2, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)(5 * sizeof(float))};
    
    model = RendererCreateModelAOS(vertices_data, v_idx * sizeof(GLfloat), indices_data, indices_written * sizeof(GLint), attribs, 3, vertex_shader_path, fragment_shader_path);

    model.indices_count = indices_written;

    return model;
}

Model_blueprint *RendererCreateModel(Assimp_object ass, char *vertex_shader_path, char *fragment_shader_path)
{
    Model_blueprint *models;
    models = malloc(sizeof(Model_blueprint) * ass.n_meshes);

    VAOAttribute *attribs = VAOCreateVAOAttributeArrays(3);
    attribs[0] = (VAOAttribute){0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)(0 * sizeof(float))};
    attribs[1] = (VAOAttribute){1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)(3 * sizeof(float))};
    attribs[2] = (VAOAttribute){2, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void *)(5 * sizeof(float))};

    printf("Starting to create model\n");
    
    for (size_t i = 0; i < ass.n_meshes; i++)
    {
        printf("Creating mesh %d\n", i);
        models[i] = RendererCreateModelAOS(
            ass.meshes[i].vertices,
            ass.meshes[i].vertex_count * sizeof(GLfloat),
            ass.meshes[i].indices,
            ass.meshes[i].index_count * sizeof(GLuint),
            attribs, 3, vertex_shader_path, fragment_shader_path
        );
        
        models[i].indices_count = ass.meshes[i].index_count;

        printf("Created mesh\n");
    }

    printf("Model creater indices check: no of indices=%d, first index=%d\n", models[0].indices_count, models[0].indices[0]);

    printf("Created array of meshes\n");
    return models;
}

void DeleteModels(Model_blueprint m)
{
    glDeleteVertexArrays(1, &m.VAO);
    glDeleteBuffers(1, &m.VBO);
    glDeleteBuffers(1, &m.EBO);
    glDeleteTextures(1, &m.texture);
    glDeleteProgram(m.shader_program);
}

void RendererCopyVec3ToModel(Model_blueprint *m, vec3 vector3)
{
    for (size_t i = 0; i < 3; i++)
    {
        m->position[i] = vector3[i];
    }

    glm_translate(m->model, m->position);
}