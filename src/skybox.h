#pragma once
#include "vector.h"
#include "triangle.h"
#include "mesh.h"
#include <vector>
#include <string>
#define N_CUBE_VERTICES 8
#define N_CUBE_FACES 12 // 6 cube faces, 2 triangles per face

extern vec3_t skybox_vertices[N_CUBE_VERTICES];
extern face_t skybox_faces[N_CUBE_FACES];

void loadskybox_data(mesh_t& mesh);
