#ifndef MESH_H
#define MESH_H

#include "vector.h"
#include "triangle.h"
#include <vector>
#include <string>
#define N_CUBE_VERTICES 8
#define N_CUBE_FACES (6 * 2) // 6 cube faces, 2 triangles per face

extern vec3_t cube_vertices[N_CUBE_VERTICES];
extern face_t cube_faces[N_CUBE_FACES];

////////////////////////////////////////////////////////////////////////////////
// Define a struct for dynamic size meshes, with array of vertices and faces
////////////////////////////////////////////////////////////////////////////////
typedef struct {
    std::vector<vec3_t> vertices;   // dynamic array of vertices
    std::vector<face_t> faces;      // dynamic array of faces
    vec3_t rotation;    // rotation with x, y, and z values
    vec3_t scale;       // scale with x, y, and z values
    vec3_t translation; // translation with x, y, and z values
} mesh_t;

extern mesh_t mesh;

void load_cube_mesh_data(void);
void load_obj_file_data(std::string filename);
void load_textured_obj(std::string name);
#endif