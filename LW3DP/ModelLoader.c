#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "headers/ModelLoader.h"

void ProcessAssimpMesh(struct aiMesh *mesh, const struct aiScene *scene, 
                      Assimp_mesh **mesh_list, int mesh_idx, unsigned int base_vertex)
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

    // 5. CORRECT ASSIGNMENT
    (*mesh_list)[mesh_idx] = *a_mesh;
    free(a_mesh);
}

Assimp_object LoadAssimp(const char *fname)
{
    const struct aiScene *scene = aiImportFile(fname, 
        aiProcess_Triangulate |
        aiProcess_FlipUVs | aiProcess_CalcTangentSpace |
        aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices
    );

    Assimp_object ass;
    Assimp_mesh *meshes = malloc( sizeof(Assimp_mesh) * scene->mNumMeshes );

    printf("Starting to load model\n");

    unsigned int base_vertex = 0;
    for (size_t i = 0; i < scene->mNumMeshes; i++)
    {
        ProcessAssimpMesh(scene->mMeshes[i], scene, &meshes, i, base_vertex);
        base_vertex += scene->mMeshes[i]->mNumVertices;
    }

    ass.meshes = meshes;
    ass.n_meshes = scene->mNumMeshes;

    printf("Loader indices check: no of indices=%d, first index=%d\n", meshes[0].index_count, meshes[0].indices[0]);
    
    aiReleaseImport(scene);

    return ass;
}


char* ParseMTLOptions(char *line, OBJ_Material *mat) {
    if (!line || *line == '\0') return NULL;

    int sscanf_chars_read;    
    char *ptr = line;
    
    // Skip leading whitespace
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    // Parse through options
    while (*ptr) {
        // If we encounter an option flag
        if (*ptr == '-') {
            ptr++; // skip the '-'
            
            // Handle different options
            if (*ptr == 's') { // -s scale option
                ptr++; // skip 's'
                sscanf(ptr, " %f %f %f%n", &mat->texture_scale[0], &mat->texture_scale[1], &mat->texture_scale[2], &sscanf_chars_read);
                ptr+=sscanf_chars_read;
            }
            else if (*ptr == 't') { // -t turbulence option
                ptr++; // skip 't'
                // Skip turbulence values (three floats)
                for (int i = 0; i < 3; i++) {
                    while (*ptr == ' ' || *ptr == '\t') ptr++;
                    while (*ptr && *ptr != ' ' && *ptr != '\t') ptr++;
                }
            }
            else if (*ptr == 'o') { // -o offset option
                ptr++; // skip 'o'
                // Skip offset values (three floats)
                for (int i = 0; i < 3; i++) {
                    while (*ptr == ' ' || *ptr == '\t') ptr++;
                    while (*ptr && *ptr != ' ' && *ptr != '\t') ptr++;
                }
            }
            else if (*ptr == 'b' && *(ptr+1) == 'm') { // -bm bump multiplier
                ptr += 2; // skip 'bm'
                // Skip bump multiplier value (one float)
                while (*ptr == ' ' || *ptr == '\t') ptr++;
                while (*ptr && *ptr != ' ' && *ptr != '\t') ptr++;
            }
            else {
                // Unknown option, skip it
                while (*ptr && *ptr != ' ' && *ptr != '\t') ptr++;
            }
            
            // Skip whitespace after option
            while (*ptr == ' ' || *ptr == '\t') ptr++;
        } else {
            // We've reached the texture path
            break;
        }
    }
    
    // Trim leading whitespace from texture path
    while (*ptr == ' ' || *ptr == '\t') ptr++;
    
    return (*ptr != '\0') ? ptr : NULL;
}

OBJ_Material *ReadMTLFile(const char *fname, int buffer_size, int *out_count)
{
    FILE *f = fopen(fname, "r");
    if (!f) {
        perror("fopen");
        return NULL;
    }

    char *buffer = malloc(buffer_size);
    if (!buffer) {
        perror("malloc");
        fclose(f);
        return NULL;
    }

    OBJ_Material *mats  = NULL;
    int           nmats = 0;
    int           cur   = -1;          /* no current material yet */

    while (fscanf(f, "%s", buffer) == 1) {
        if (strcmp(buffer, "newmtl") == 0) {
            /* ---- new material ---- */
            mats = realloc(mats, sizeof(*mats) * (nmats + 1));
            if (!mats) { perror("realloc"); exit(EXIT_FAILURE); }

            cur = nmats++;
            OBJ_Material *m = &mats[cur];

            *m = (OBJ_Material){0};               /* zero everything */
            if (fscanf(f, " %ms", &m->name) != 1) /* %m allocates the string */
                m->name = strdup("unknown");

        } else if (cur == -1) {
            /* ignore any line that appears before the first newmtl */
            continue;

        } else if (strcmp(buffer, "Ka") == 0) {
            fscanf(f, " %f %f %f", &mats[cur].Ka[0], &mats[cur].Ka[1], &mats[cur].Ka[2]);

        } else if (strcmp(buffer, "Kd") == 0) {
            fscanf(f, " %f %f %f", &mats[cur].Kd[0], &mats[cur].Kd[1], &mats[cur].Kd[2]);

        } else if (strcmp(buffer, "Ks") == 0) {
            fscanf(f, " %f %f %f", &mats[cur].Ks[0], &mats[cur].Ks[1], &mats[cur].Ks[2]);

        } else if (strcmp(buffer, "d") == 0) {
            fscanf(f, " %f", &mats[cur].d_Tr);

        } else if (strcmp(buffer, "map_Kd") == 0) {
            // Read the entire line
            char line[1024];
            if (fgets(line, sizeof(line), f)) {
                // Remove trailing newline
                line[strcspn(line, "\n")] = 0;
                
                // Parse options and find the actual texture path
                char *texture_path = ParseMTLOptions(line, &mats[cur]);
                if (texture_path && *texture_path != '\0') {
                    mats[cur].texture_path = strdup(texture_path);
                    printf("Texture path: '%s'\n", mats[cur].texture_path);
                }
            }
        } else {
            fgets(buffer, buffer_size, f);
        }
    }

    free(buffer);
    fclose(f);
    *out_count = nmats;
    return mats;
}

OBJ_face *LoadOBJ(const char *fname, int buffer_length, int *n_faces)
{
    FILE *f = fopen(fname, "r");
    if (f == NULL) {
        printf("Error: Cannot open file %s\n", fname);
        return (OBJ_face *)NULL;
    }
    
    char *buffer = malloc(sizeof(char) * buffer_length);
    if (buffer == NULL) {
        fclose(f);
        return (OBJ_face *)NULL;
    }

    // Initialize to NULL
    GLfloat *vertices = NULL;
    GLfloat *normals = NULL;
    GLfloat *texcoords = NULL;
    GLint *indices = NULL;

    int vertices_a = 0, normals_a = 0, texcoords_a = 0;

    OBJ_Material *mats = NULL;
    OBJ_face *faces = NULL;
    OBJ_face **objects = NULL;
    int nobjects = 0;
    int nfaces = 0;

    while (fscanf(f, "%s", buffer) != EOF)
    {
        if (!strcmp("v", buffer))
        {
            GLfloat *temp = realloc(vertices, (vertices_a + 3) * sizeof(GLfloat));
            if (temp == NULL) {
                printf("Memory allocation failed for vertices\n");
                break;
            }
            vertices = temp;
            
            if (fscanf(f, " %f %f %f\n", &vertices[vertices_a], 
                                         &vertices[vertices_a+1], 
                                         &vertices[vertices_a+2]) == 3) {
                vertices_a += 3;
            } else {
                printf("Error reading vertex data\n");
            }
        }
        else if (!strcmp("vt", buffer))
        {
            GLfloat *temp = realloc(texcoords, (texcoords_a + 2) * sizeof(GLfloat));
            if (temp == NULL) {
                printf("Memory allocation failed for texcoords\n");
                break;
            }
            texcoords = temp;
            
            if (fscanf(f, " %f %f\n", &texcoords[texcoords_a], 
                                      &texcoords[texcoords_a+1]) == 2) {
                texcoords_a += 2;  // CORRECTED: increment counter, not pointer
            } else {
                printf("Error reading texture data\n");
            }
        }
        else if (!strcmp("vn", buffer))
        {
            GLfloat *temp = realloc(normals, (normals_a + 3) * sizeof(GLfloat));
            if (temp == NULL) {
                printf("Memory allocation failed for normals\n");
                break;
            }
            normals = temp;
            
            if (fscanf(f, " %f %f %f\n", &normals[normals_a], 
                                         &normals[normals_a+1], 
                                         &normals[normals_a+2]) == 3) {
                normals_a += 3;
            } else {
                printf("Error reading normal data\n");
            }
        }
        else if (!strcmp("f", buffer))
        {
            unsigned int vertexIndex[4], uvIndex[4], normalIndex[4];
            int scanned = fscanf(f, " %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                &vertexIndex[0], &uvIndex[0], &normalIndex[0],
                &vertexIndex[1], &uvIndex[1], &normalIndex[1], 
                &vertexIndex[2], &uvIndex[2], &normalIndex[2],
                &vertexIndex[3], &uvIndex[3], &normalIndex[3]
            );
            
            if (scanned == 9)
            {
                // Realloc faces array
                OBJ_face *temp = realloc(faces, sizeof(OBJ_face) * (nfaces + 1));
                if (temp == NULL) {
                    printf("Memory allocation failed for faces\n");
                    break;
                }
                faces = temp;
                
                // Initialize the new face
                OBJ_face *face = &faces[nfaces];
                
                // For each vertex in the face (3 vertices per triangle)
                for (int i = 0; i < 3; i++) {
                    // Convert OBJ indices (1-based) to array indices (0-based)
                    int v_idx = (vertexIndex[i] - 1) * 3;
                    int uv_idx = (uvIndex[i] - 1) * 2;
                    int n_idx = (normalIndex[i] - 1) * 3;
                    
                    // Make sure indices are within bounds
                    if (v_idx >= 0 && v_idx < vertices_a &&
                        uv_idx >= 0 && uv_idx < texcoords_a && 
                        n_idx >= 0 && n_idx < normals_a)
                        {
                        
                        // Store position (x, y, z)
                        face->vertex[i*3]     = vertices[v_idx];
                        face->vertex[i*3 + 1] = vertices[v_idx + 1]; 
                        face->vertex[i*3 + 2] = vertices[v_idx + 2];
                        
                        // Store texture coordinates (u, v)
                        face->texture_coord[i*2]     = texcoords[uv_idx];
                        face->texture_coord[i*2 + 1] = texcoords[uv_idx + 1];
                        
                        // Store normal (nx, ny, nz)
                        face->normal[i*3]     = normals[n_idx];
                        face->normal[i*3 + 1] = normals[n_idx + 1];
                        face->normal[i*3 + 2] = normals[n_idx + 2];
                    }
                }
                
                nfaces++;  // IMPORTANT: Increment face count
            } else
             if (scanned == 12) // Quad face - convert to 2 triangles
            {
                // Realloc faces array for 2 new triangles
                OBJ_face *temp = realloc(faces, sizeof(OBJ_face) * (nfaces + 2));
                if (temp == NULL) {
                    printf("Memory allocation failed for faces\n");
                    break;
                }
                faces = temp;

                // First triangle: vertices 0, 1, 2
                OBJ_face *face1 = &faces[nfaces];
                int tri1_indices[3] = {0, 1, 2};
                
                for (int i = 0; i < 3; i++) {
                    int vertex_idx = tri1_indices[i];
                    
                    // Convert OBJ indices (1-based) to array indices (0-based)
                    int v_idx = (vertexIndex[vertex_idx] - 1) * 3;
                    int uv_idx = (uvIndex[vertex_idx] - 1) * 2;
                    int n_idx = (normalIndex[vertex_idx] - 1) * 3;
                    
                    // Make sure indices are within bounds
                    if (v_idx >= 0 && v_idx < vertices_a &&
                        uv_idx >= 0 && uv_idx < texcoords_a &&
                        n_idx >= 0 && n_idx < normals_a)
                    {
                        // Store position (x, y, z)
                        face1->vertex[i*3]     = vertices[v_idx];
                        face1->vertex[i*3 + 1] = vertices[v_idx + 1];
                        face1->vertex[i*3 + 2] = vertices[v_idx + 2];
                        
                        // Store texture coordinates (u, v)
                        face1->texture_coord[i*2]     = texcoords[uv_idx];
                        face1->texture_coord[i*2 + 1] = texcoords[uv_idx + 1];
                        
                        // Store normal (nx, ny, nz)
                        face1->normal[i*3]     = normals[n_idx];
                        face1->normal[i*3 + 1] = normals[n_idx + 1];
                        face1->normal[i*3 + 2] = normals[n_idx + 2];
                    }
                }

                // Second triangle: vertices 0, 2, 3
                OBJ_face *face2 = &faces[nfaces + 1];
                int tri2_indices[3] = {0, 2, 3};
                
                for (int i = 0; i < 3; i++) {
                    int vertex_idx = tri2_indices[i];
                    
                    // Convert OBJ indices (1-based) to array indices (0-based)
                    int v_idx = (vertexIndex[vertex_idx] - 1) * 3;
                    int uv_idx = (uvIndex[vertex_idx] - 1) * 2;
                    int n_idx = (normalIndex[vertex_idx] - 1) * 3;
                    
                    // Make sure indices are within bounds
                    if (v_idx >= 0 && v_idx < vertices_a &&
                        uv_idx >= 0 && uv_idx < texcoords_a &&
                        n_idx >= 0 && n_idx < normals_a)
                    {
                        // Store position (x, y, z)
                        face2->vertex[i*3]     = vertices[v_idx];
                        face2->vertex[i*3 + 1] = vertices[v_idx + 1];
                        face2->vertex[i*3 + 2] = vertices[v_idx + 2];
                        
                        // Store texture coordinates (u, v)
                        face2->texture_coord[i*2]     = texcoords[uv_idx];
                        face2->texture_coord[i*2 + 1] = texcoords[uv_idx + 1];
                        
                        // Store normal (nx, ny, nz)
                        face2->normal[i*3]     = normals[n_idx];
                        face2->normal[i*3 + 1] = normals[n_idx + 1];
                        face2->normal[i*3 + 2] = normals[n_idx + 2];
                    }
                }
                
                nfaces += 2;
            }
            else
            {
                printf("Face format not supported\n");
                printf("%s\n", buffer);
                exit(0);
                fgets(buffer, buffer_length, f); // Skip the rest of the line
            }
        }
        else if (!strcmp("mtllib", buffer))
        {
            char *mtl_fname;
            fscanf(f, " %ms", mtl_fname);

            int nmats;
            mats = ReadMTLFile(mtl_fname, 2048, &nmats);
            printf("Read the mtl file\n");
        }
        else if (!strcmp("usemtl", buffer))
        {
            nobjects++;
            // objects = realloc(objects, sizeof(OBJ_object) * nobjects);
            fgets(buffer, buffer_length, f);
        }
        else { 
            fgets(buffer, buffer_length, f); // Skip the rest of the line
        }
    }

    // Cleanup
    free(vertices);
    free(normals);
    free(texcoords);
    free(indices);
    free(buffer);
    fclose(f);

    *n_faces = nfaces;

    printf("loaded %d vertices\n", nfaces * 8);
    
    printf("VERTEX %d DATA %f %f %f %f %f %f %f %f\n", 1, faces[1].vertex[0], faces[1].vertex[1]
                                                    , faces[1].vertex[2], faces[1].texture_coord[0], faces[1].texture_coord[1]
                                                    , faces[1].normal[0], faces[1].normal[1], faces[1].normal[2]);

    return faces;
}