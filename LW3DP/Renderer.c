#include "headers/Renderer.h"

#include "headers/Textures.h"

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
        models[i] = RendererCreateModelAOS(
            ass.meshes[i].vertices,
            ass.meshes[i].vertex_count * sizeof(GLfloat),
            ass.meshes[i].indices,
            ass.meshes[i].index_count * sizeof(GLuint),
            attribs, 3, vertex_shader_path, fragment_shader_path
        );
        
        models[i].indices_count = ass.meshes[i].index_count;
        models[i].model_name = ass.meshes[i].mesh_name;

        if (ass.meshes[i].texture.has_texture)
        {
            models[i].texture = ass.meshes[i].texture.texture_id;

            printf("RendererCreateModel() texture path [%s]\n", ass.meshes[i].texture.texture_path);
            TextureCreateTexture(&models[i].texture, models[i].shader_program, "tex0", 0, ass.meshes[i].texture.texture_path);
        } else
        {
            models[i].texture = 0;
        }
    }
    return models;
}

Model_blueprint RendererCreateLight(char *light_vertex_shader, char *light_fragment_shader, vec4 light_color)
{
    GLfloat lightVertices[] =
    { //     COORDINATES     //
        -0.1f, -0.1f,  0.1f,
        -0.1f, -0.1f, -0.1f,
        0.1f, -0.1f, -0.1f,
        0.1f, -0.1f,  0.1f,
        -0.1f,  0.1f,  0.1f,
        -0.1f,  0.1f, -0.1f,
        0.1f,  0.1f, -0.1f,
        0.1f,  0.1f,  0.1f
    };

    GLuint lightIndices[] =
    {
        0, 1, 2,
        0, 2, 3,
        0, 4, 7,
        0, 7, 3,
        3, 7, 6,
        3, 6, 2,
        2, 6, 5,
        2, 5, 1,
        1, 5, 4,
        1, 4, 0,
        4, 5, 6,
        4, 6, 7
    };

    VAOAttribute *t_attrs = VAOCreateVAOAttributeArrays(1);
    t_attrs[0] = (VAOAttribute){0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void *)0};
    Model_blueprint c = RendererCreateModelAOS(
        lightVertices, sizeof(lightVertices), lightIndices, sizeof(lightIndices),
        t_attrs, 1, light_vertex_shader, light_fragment_shader
    );
    free(t_attrs);

    RendererCopyVec3ToModel(&c, (vec3){-1.2f, 1.0f, 1.0f});
    UniformSend4x4Matrix(c.shader_program, "model", c.model);

    glm_vec4_copy3(light_color, c.color);

    return c;
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