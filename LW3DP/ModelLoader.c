#include "headers/ModelLoader.h"

int mesh_idx = 0;

void ProcessAssimpMesh(struct aiMesh *mesh, const struct aiScene *scene, 
                      Assimp_mesh **mesh_list, mat4 transform)
{
    Assimp_mesh *a_mesh = &(*mesh_list)[mesh_idx];
    int ptr = 0;    

    // 1. ALLOCATE MEMORY for vertices
    int vertex_size = 8; // 3 pos + 2 tex + 3 normal
    int total_vertices = mesh->mNumVertices * vertex_size;
    
    a_mesh->vertices = (GLfloat*)malloc(sizeof(GLfloat) * total_vertices);
    
    if (!a_mesh->vertices) {
        printf("ERROR: Failed to allocate memory for vertices\n");
        return;
    }

    // 2. PROCESS VERTICES
    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        // Position
        vec3 v = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        //glm_mat4_mulv3(transform, v, 1.0f, v);

        a_mesh->vertices[ptr++] = v[0];
        a_mesh->vertices[ptr++] = v[1];
        a_mesh->vertices[ptr++] = v[2];

        // Texture coordinates
        if (mesh->mTextureCoords[0]) {
            a_mesh->vertices[ptr++] = mesh->mTextureCoords[0][i].x;
            a_mesh->vertices[ptr++] = mesh->mTextureCoords[0][i].y;
        } else {
            a_mesh->vertices[ptr++] = 0.0f;
            a_mesh->vertices[ptr++] = 0.0f;
        }

        // Normals
        if (mesh->mNormals) {
            mat3 normal_transform;
            
            for (size_t k = 0; k < 3; k++)
            {
                for (size_t j = 0; j < 3; j++)
                {
                    normal_transform[k][j] = transform[k][j];
                }
            }

            glm_mat3_inv(normal_transform, normal_transform);
            glm_mat3_transpose(normal_transform);

            vec3 v = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};

            //glm_mat3_mulv(normal_transform, v, v);

            a_mesh->vertices[ptr++] = v[0];
            a_mesh->vertices[ptr++] = v[1];
            a_mesh->vertices[ptr++] = v[2];
        } else {
            a_mesh->vertices[ptr++] = 0.0f;
            a_mesh->vertices[ptr++] = 0.0f;
            a_mesh->vertices[ptr++] = 0.0f;
        }
    }

    a_mesh->vertex_count = ptr;
    
    // 3. PROCESS INDICES
    a_mesh->index_count = mesh->mNumFaces * 3; // Assuming triangulated
    a_mesh->indices = (unsigned int*)malloc(sizeof(unsigned int) * a_mesh->index_count);
    
    if (a_mesh->indices) {
        unsigned int index_ptr = 0;

        for (size_t i = 0; i < mesh->mNumFaces; i++) {
            a_mesh->indices[index_ptr++] = mesh->mFaces[i].mIndices[0];
            a_mesh->indices[index_ptr++] = mesh->mFaces[i].mIndices[1];
            a_mesh->indices[index_ptr++] = mesh->mFaces[i].mIndices[2];
        }
    }

    a_mesh->mesh_name = malloc(sizeof(const char) * mesh->mName.length);
    a_mesh->mesh_name = strdup(mesh->mName.data);
    a_mesh->identifier = mesh_idx;

    mesh_idx++;
}

void convert_backslash_to_frontslash(char *str, int len)
{
    for (size_t i = 0; i < len; i++)
    {
        if (str[i] == '\\')
        {
            str[i] = '/';
        }
    }
}

void ProcessMaterialTextures(struct aiMaterial **materials, struct aiMesh *mesh, Assimp_mesh *created_mesh, char *model_path, const struct aiScene *scene)
{
    const struct aiMaterial *mat = materials[mesh->mMaterialIndex];
    struct aiString path;
    aiReturn ok;

    ok = aiGetMaterialTexture(
        mat, aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL, NULL
    );

    if (ok == AI_SUCCESS)
    {
        if (path.data[0] == '*')
        {
            printf("Embedded texture\n");
            created_mesh->texture.embedded = 1;

            int idx = atoi(path.data+1);

            if (idx >= 0 && idx < scene->mNumTextures)
            {
                struct aiTexture *tex = scene->mTextures[idx];

                created_mesh->texture.embedded_data = (unsigned char *)tex->pcData;
                created_mesh->texture.embedded_len = tex->mWidth;

                printf("Created embedded texture %d, data length %d\n", idx, created_mesh->texture.embedded_len);
            } else
            {
                printf("Embedded texture idx out of range\n");
            }
        } else
        {
            printf("External texture\n");
            created_mesh->texture.texture_id = mesh->mMaterialIndex;
            created_mesh->texture.embedded = 0;

            char actual_path[4096];
            strcpy(actual_path, model_path);

            strcat(actual_path, "\\");
            strcat(actual_path, path.data);

            convert_backslash_to_frontslash(actual_path, 4096);

            created_mesh->texture.texture_path = strdup(actual_path);
            created_mesh->texture.has_texture = 1;
        }
    } else
    {
        printf("mesh %d has no diffuse texture\n", created_mesh->identifier);
        created_mesh->texture.has_texture = 0;
    }

    ok = aiGetMaterialTexture(
        mat, aiTextureType_UNKNOWN, 0, &path, NULL, NULL, NULL, NULL, NULL, NULL
    );

    if (ok = AI_SUCCESS)
    {
        printf("Unknown texture found for mesh %d\n", created_mesh->identifier);
    }

    ok = aiGetMaterialTexture(
        mat, aiTextureType_SPECULAR, 0, &path, NULL, NULL, NULL, NULL, NULL, NULL
    );

    if (ok = AI_SUCCESS)
    {
        printf("Specular texture found for mesh %d\n", created_mesh->identifier);
    } else
    {
        printf("mesh %d has no specular texture\n", created_mesh->identifier);
    }
    
    ok = aiGetMaterialTexture(
        mat, aiTextureType_NORMALS, 0, &path, NULL, NULL, NULL, NULL, NULL, NULL
    );

    if (ok = AI_SUCCESS)
    {
        printf("Normal texture found for mesh %d\n", created_mesh->identifier);
    } else
    {
        printf("mesh %d has no normal texture\n", created_mesh->identifier);
    }

    ok = aiGetMaterialTexture(
        mat, aiTextureType_HEIGHT, 0, &path, NULL, NULL, NULL, NULL, NULL, NULL
    );
    
    if (ok = AI_SUCCESS)
    {
        printf("Height texture found for mesh %d\n", created_mesh->identifier);
    } else
    {
        printf("mesh %d has no height texture\n", created_mesh->identifier);
    }

    ok = aiGetMaterialTexture(
        mat, aiTextureType_LIGHTMAP, 0, &path, NULL, NULL, NULL, NULL, NULL, NULL
    );
    
    if (ok = AI_SUCCESS)
    {
        printf("Lightmap texture found for mesh %d\n", created_mesh->identifier);
    } else
    {
        printf("mesh %d has no lightmap texture\n", created_mesh->identifier);
    }
}


static inline void aimat4x4_to_cglm(const struct aiMatrix4x4 m, mat4 out)
{
    out[0][0] = m.a1; out[0][1] = m.b1; out[0][2] = m.c1; out[0][3] = m.d1;
    out[1][0] = m.a2; out[1][1] = m.b2; out[1][2] = m.c2; out[1][3] = m.d2;
    out[2][0] = m.a3; out[2][1] = m.b3; out[2][2] = m.c3; out[2][3] = m.d3;
    out[3][0] = m.a4; out[3][1] = m.b4; out[3][2] = m.c4; out[3][3] = m.d4;
}

void ProcessAssimpNode(const struct aiScene *scene, const struct aiNode *node, mat4 parentMat, Assimp_mesh **meshes)
{
    mat4 local, world;

    aimat4x4_to_cglm(node->mTransformation, local);
    glm_mat4_mul(parentMat, local, world);

    for (size_t i = 0; i < node->mNumMeshes; i++)
    {
        ProcessAssimpMesh(scene->mMeshes[node->mMeshes[i]], scene, meshes, world);
    }
    
    for (size_t i = 0; i < node->mNumChildren; i++)
    {
        ProcessAssimpNode(scene, node->mChildren[i], world, meshes);
    }
}

Assimp_object LoadAssimp(const char *fname)
{
    const struct aiScene *scene = aiImportFile(fname, 
        aiProcess_Triangulate |
        aiProcess_CalcTangentSpace |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_FlipUVs
    );

    Assimp_object ass;
    Assimp_mesh *meshes = malloc( sizeof(Assimp_mesh) * scene->mNumMeshes );

    char model_path[PATH_MAX];

    strcpy(model_path, fname);
    PathRemoveFileSpecA(model_path);

    mat4 test_matrice;
    glm_mat4_identity(test_matrice);
    // glm_rotate_y(test_matrice, glm_rad(90), test_matrice);

    ProcessAssimpNode(scene, scene->mRootNode, test_matrice, &meshes);

    for (size_t i = 0; i < scene->mNumMeshes; i++)
    {
        //ProcessAssimpMesh(scene->mMeshes[i], scene, &meshes, test_matrice);
        ProcessMaterialTextures(scene->mMaterials, scene->mMeshes[i], &meshes[i], model_path, scene);
    }

    ass.meshes = meshes;
    ass.n_meshes = scene->mNumMeshes;
    
    aiReleaseImport(scene);

    return ass;
}