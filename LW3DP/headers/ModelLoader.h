#ifndef MODELLOADER_H
#define MODELLOADER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Textures.h"

void LW3DP_Loader(char *path, int line_buffer_size);
OBJ_face *LoadOBJ(const char *fname, int buffer_length, int *n_faces);
Assimp_object LoadAssimp(const char *fname);
#endif