#include "headers/ModelLoader.h"

#define MAX_BONES_AMOUNT 4 // per vertex value

int mesh_idx = 0;
mat4 global_inv_transf;

static inline void aimat4x4_to_cglm(const struct aiMatrix4x4 *m, mat4 out)
{
    // Column 0
    out[0][0] = m->a1; out[0][1] = m->a2; out[0][2] = m->a3; out[0][3] = m->a4;
    // Column 1  
    out[1][0] = m->b1; out[1][1] = m->b2; out[1][2] = m->b3; out[1][3] = m->b4;
    // Column 2
    out[2][0] = m->c1; out[2][1] = m->c2; out[2][2] = m->c3; out[2][3] = m->c4;
    // Column 3
    out[3][0] = m->d1; out[3][1] = m->d2; out[3][2] = m->d3; out[3][3] = m->d4;
}


void ProcessAssimpBones(const struct aiScene *scene, const struct aiMesh *mesh, unsigned int vertex_id, Assimp_bone *bone, Assimp_skeleton skeleton)
{
    int n = 0;
    mat4 cglm_offset_matrix;
    
    for (float x = 0; x < mesh->mNumBones; x++)
    {
        int i = x;
        for (size_t j = 0; j < mesh->mBones[i]->mNumWeights; j++)
        {
            if (mesh->mBones[i]->mWeights[j].mVertexId == vertex_id)
            {
                if (n == MAX_BONES_AMOUNT) { break; }
                (bone->weights)[n] = mesh->mBones[i]->mWeights[j].mWeight;
                (bone->bone_ids)[n] = x;
                aimat4x4_to_cglm(&mesh->mBones[i]->mOffsetMatrix, cglm_offset_matrix);
                glm_mat4_copy(cglm_offset_matrix, bone->offset_matrix);
                n++;
                break;
            }
        }
        if (n == MAX_BONES_AMOUNT) { break; }
    }
    //printf("Processed %d bones\n", mesh->mNumBones);
}

void ProcessAssimpMesh(struct aiMesh *mesh, const struct aiScene *scene, 
                      Assimp_mesh **mesh_list, mat4 transform, Assimp_skeleton skeleton)
{
    Assimp_mesh *a_mesh = &(*mesh_list)[mesh_idx];
    int ptr = 0;    

    // 1. ALLOCATE MEMORY for vertices
    int vertex_size = 16; // 3 pos + 2 tex + 3 normal
    int total_vertices = mesh->mNumVertices * vertex_size;
    
    a_mesh->vertices = (GLfloat*)malloc(sizeof(GLfloat) * total_vertices);
    
    if (!a_mesh->vertices) {
        printf("ERROR: Failed to allocate memory for vertices\n");
        return;
    }

    a_mesh->bones = (Assimp_bone*)malloc(sizeof(Assimp_bone) * mesh->mNumVertices);

    if (!a_mesh->bones) {
        printf("ERROR: Failed to allocate memory for bones\n");
        return;
    }

    printf("Starting to process vertices\n");

    // 2. PROCESS VERTICES
    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        // Position
        a_mesh->vertices[ptr++] = mesh->mVertices[i].x;
        a_mesh->vertices[ptr++] = mesh->mVertices[i].y;
        a_mesh->vertices[ptr++] = mesh->mVertices[i].z;

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
            a_mesh->vertices[ptr++] = mesh->mNormals[i].x;
            a_mesh->vertices[ptr++] = mesh->mNormals[i].y;
            a_mesh->vertices[ptr++] = mesh->mNormals[i].z;
        } else {
            a_mesh->vertices[ptr++] = 0.0f;
            a_mesh->vertices[ptr++] = 0.0f;
            a_mesh->vertices[ptr++] = 0.0f;
        }
        
        // Bone IDS
        a_mesh->vertices[ptr++] = 0.0f;
        a_mesh->vertices[ptr++] = 0.0f;
        a_mesh->vertices[ptr++] = 0.0f;
        a_mesh->vertices[ptr++] = 0.0f;

        // Bone Weights
        a_mesh->vertices[ptr++] = 0.0f;
        a_mesh->vertices[ptr++] = 0.0f;
        a_mesh->vertices[ptr++] = 0.0f;
        a_mesh->vertices[ptr++] = 0.0f;
        
        if (mesh->mNumBones > 0)
        {
            a_mesh->bones[i].no_of_weights = MAX_BONES_AMOUNT;
            a_mesh->bones[i].weights = calloc(MAX_BONES_AMOUNT, sizeof(float));
            a_mesh->bones[i].bone_ids = calloc(MAX_BONES_AMOUNT, sizeof(float));
            ProcessAssimpBones(scene, mesh, i, &a_mesh->bones[i], skeleton);

            a_mesh->vertices[ptr-8] = a_mesh->bones[i].bone_ids[0];
            a_mesh->vertices[ptr-7] = a_mesh->bones[i].bone_ids[1];
            a_mesh->vertices[ptr-6] = a_mesh->bones[i].bone_ids[2];
            a_mesh->vertices[ptr-5] = a_mesh->bones[i].bone_ids[3];

            // Bone Weights
            a_mesh->vertices[ptr-4] = a_mesh->bones[i].weights[0];
            a_mesh->vertices[ptr-3] = a_mesh->bones[i].weights[1];
            a_mesh->vertices[ptr-2] = a_mesh->bones[i].weights[2];
            a_mesh->vertices[ptr-1] = a_mesh->bones[i].weights[3];
        }
    }
    //printf("Processed vertices\n");

    mat4 *bone_offset_matrices = calloc(100, sizeof(mat4));

    for (size_t i = 0; i < mesh->mNumBones; i++)
    {
        mat4 offset_matrix;
        aimat4x4_to_cglm(&mesh->mBones[i]->mOffsetMatrix, offset_matrix);

        glm_mat4_copy(offset_matrix, bone_offset_matrices[i]);
    }
    
    a_mesh->bone_offset_matrices = calloc(100, sizeof(mat4));
    a_mesh->bone_offset_matrices = bone_offset_matrices;

    a_mesh->vertex_count = ptr;

    glm_mat4_identity(a_mesh->transformation);
    glm_mat4_copy(transform, a_mesh->transformation);
    
    // 3. PROCESS INDICES
    a_mesh->index_count = mesh->mNumFaces * 3;
    a_mesh->indices = (unsigned int*)malloc(sizeof(unsigned int) * a_mesh->index_count);
    
    if (a_mesh->indices) {
        unsigned int index_ptr = 0;

        for (size_t i = 0; i < mesh->mNumFaces; i++) {
            a_mesh->indices[index_ptr++] = mesh->mFaces[i].mIndices[0];
            a_mesh->indices[index_ptr++] = mesh->mFaces[i].mIndices[1];
            a_mesh->indices[index_ptr++] = mesh->mFaces[i].mIndices[2];
        }
    }

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
}

void ProcessMaterialColors(struct aiMaterial **materials, struct aiMesh *mesh, Assimp_mesh *created_mesh, char *model_path, const struct aiScene *scene)
{
    const struct aiMaterial *mat = materials[mesh->mMaterialIndex];
    struct aiString path;
    aiReturn ok;

    
}

void ProcessAssimpNode(const struct aiScene *scene, const struct aiNode *node, mat4 parentMat, Assimp_mesh **meshes, Assimp_skeleton skeleton)
{
    mat4 local, world; glm_mat4_identity(local); glm_mat4_identity(world);
    struct aiMatrix4x4 world_Ai;

    aimat4x4_to_cglm(&node->mTransformation, local);

    //glm_mat4_mul(parentMat, local, world);
    //glm_mat4_mul(global_inv_transf, world, world);

    for (size_t i = 0; i < node->mNumMeshes; i++)
    {
        ProcessAssimpMesh(scene->mMeshes[node->mMeshes[i]], scene, meshes, world, skeleton);
    }
    
    for (size_t i = 0; i < node->mNumChildren; i++)
    {
        ProcessAssimpNode(scene, node->mChildren[i], world, meshes, skeleton);
    }
}

Assimp_object LoadAssimp(const char *fname)
{
    const struct aiScene *scene = aiImportFile(fname,
        aiProcess_Triangulate           |
        aiProcess_CalcTangentSpace      |
        aiProcess_GenSmoothNormals      |
        aiProcess_JoinIdenticalVertices |
        aiProcess_FlipUVs               
    );

    Assimp_object ass;
    Assimp_skeleton skeleton = {0};
    Assimp_mesh *meshes = malloc( sizeof(Assimp_mesh) * scene->mNumMeshes );

    char model_path[PATH_MAX];

    strcpy(model_path, fname);
    PathRemoveFileSpecA(model_path);

    mat4 test_matrice;
    glm_mat4_identity(test_matrice);
    //glm_rotate_y(test_matrice, glm_rad(90), test_matrice);

    memcpy(&global_inv_transf, &scene->mRootNode->mTransformation, sizeof(mat4));
    glm_mat4_inv(global_inv_transf, global_inv_transf);

    ProcessAssimpNode(scene, scene->mRootNode, test_matrice, &meshes, skeleton);

    printf("processed nodes\n");

    for (size_t i = 0; i < scene->mNumMeshes; i++)
    {
        ProcessMaterialTextures(scene->mMaterials, scene->mMeshes[i], &meshes[i], model_path, scene);
    }

    ass.meshes = meshes;
    ass.n_meshes = scene->mNumMeshes;
    
    aiReleaseImport(scene);

    return ass;
}