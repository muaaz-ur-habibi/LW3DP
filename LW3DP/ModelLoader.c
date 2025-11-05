#include "headers/ModelLoader.h"

void ProcessAssimpMesh(struct aiMesh *mesh, const struct aiScene *scene, 
                      Assimp_mesh **mesh_list, int mesh_idx)
{
    Assimp_mesh *a_mesh = malloc(sizeof(Assimp_mesh));
    int ptr = 0;    

    // 1. ALLOCATE MEMORY for vertices
    int vertex_size = 8; // 3 pos + 2 tex + 3 normal
    int total_vertices = mesh->mNumVertices * vertex_size;
    
    a_mesh->vertices = (GLfloat*)malloc(sizeof(GLfloat) * total_vertices);
    
    if (!a_mesh->vertices) {
        printf("ERROR: Failed to allocate memory for vertices\n");
        return;
    }

    // 2. PROCESS VERTICES with safety checks
    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        // Position (always exists)
        a_mesh->vertices[ptr++] = mesh->mVertices[i].x;
        a_mesh->vertices[ptr++] = mesh->mVertices[i].y;
        a_mesh->vertices[ptr++] = mesh->mVertices[i].z;

        // Texture coordinates (check if they exist)
        if (mesh->mTextureCoords[0]) {
            a_mesh->vertices[ptr++] = mesh->mTextureCoords[0][i].x;
            a_mesh->vertices[ptr++] = mesh->mTextureCoords[0][i].y;
        } else {
            a_mesh->vertices[ptr++] = 0.0f;
            a_mesh->vertices[ptr++] = 0.0f;
        }

        // Normals (check if they exist)
        if (mesh->mNormals) {
            a_mesh->vertices[ptr++] = mesh->mNormals[i].x;
            a_mesh->vertices[ptr++] = mesh->mNormals[i].y;
            a_mesh->vertices[ptr++] = mesh->mNormals[i].z;
        } else {
            a_mesh->vertices[ptr++] = 0.0f;
            a_mesh->vertices[ptr++] = 0.0f; // Default up normal
            a_mesh->vertices[ptr++] = 0.0f;
        }
    }

    a_mesh->vertex_count = ptr;
    
    // 3. PROCESS INDICES (if you need them)
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

    // 5. CORRECT ASSIGNMENT
    (*mesh_list)[mesh_idx] = *a_mesh;
    free(a_mesh);
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

void ProcessMaterialTextures(struct aiMaterial **materials, struct aiMesh *mesh, Assimp_mesh *created_mesh, char *model_path)
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
            created_mesh->texture.texture_path = "";
        } else
        {
            printf("External texture\n");
            created_mesh->texture.texture_id = mesh->mMaterialIndex;

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

Assimp_object LoadAssimp(const char *fname)
{
    const struct aiScene *scene = aiImportFile(fname, 
        aiProcess_Triangulate | aiProcess_CalcTangentSpace |
        aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices
    );

    Assimp_object ass;
    Assimp_mesh *meshes = malloc( sizeof(Assimp_mesh) * scene->mNumMeshes );

    char model_path[PATH_MAX];

    strcpy(model_path, fname);
    PathRemoveFileSpecA(model_path);

    for (size_t i = 0; i < scene->mNumMeshes; i++)
    {
        ProcessAssimpMesh(scene->mMeshes[i], scene, &meshes, i);
        ProcessMaterialTextures(scene->mMaterials, scene->mMeshes[i], &meshes[i], model_path);
    }

    ass.meshes = meshes;
    ass.n_meshes = scene->mNumMeshes;
    
    aiReleaseImport(scene);

    return ass;
}