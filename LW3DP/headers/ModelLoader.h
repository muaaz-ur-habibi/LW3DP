#ifndef MODELLOADER_H
#define MODELLOADER_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>

#include "Textures.h"

Assimp_object LoadAssimp(const char *fname);
#endif