#pragma once
#include "mesh.h"

static int objFileLoaderParser(mesh_t* mesh, const char* content);
static void objFileLoaderLineParser(mesh_t* mesh, char* line, uint32_t currentLine);
static int objFileLoaderExecuteLine(mesh_t* mesh, int argc, char* argv[], uint32_t currentLine);
void loadOBJFile(mesh_t* mesh, const char* filepath);