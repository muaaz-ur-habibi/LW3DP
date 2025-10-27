#ifndef MODELLOADER_H
#define MODELLOADER_H
#include "Textures.h"

void LW3DP_Loader(char *path, int line_buffer_size);
OBJ_face *LoadOBJ(const char *fname, int buffer_length, int *n_faces);
void LoadAssimp(const char *fname);
#endif