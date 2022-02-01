#ifndef MESH_H
#define MESH_H

#include "vector.h"
//#include "upng.h"
#include <sdl_image.h>
#include "triangle.h"
#include <vector>
#include <string>

////////////////////////////////////////////////////////////////////////////////
// Define a struct for dynamic size meshes, with array of vertices and faces
////////////////////////////////////////////////////////////////////////////////
typedef struct {
    std::vector<vec3_t> vertices;   // dynamic array of vertices
    std::vector<face_t> faces;      // dynamic array of faces
    SDL_Surface* texture;
    vec3_t rotation;    // rotation with x, y, and z values
    vec3_t scale;       // scale with x, y, and z values
    vec3_t translation; // translation with x, y, and z values
} mesh_t;

extern mesh_t mesh;

void load_mesh_obj_data(mesh_t* mesh, std::string filename);
void load_mesh(std::string modelname, std::string pngname,
    vec3_t scale, vec3_t translation, vec3_t rotation);
void load_mesh(std::string name, vec3_t scale,
    vec3_t translation, vec3_t rotation);
void load_mesh_png_data(mesh_t* mesh, std::string filename);
int get_num_meshes();
mesh_t* get_mesh(int index);
#endif
