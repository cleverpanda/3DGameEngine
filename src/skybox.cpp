#include "skybox.h"
#include "mesh.h"
#include "colors.h"

vec3_t cube_vertices[N_CUBE_VERTICES] = {
    {-1,-1,-1},
    {-1, 1,-1},
    { 1, 1,-1},
    { 1,-1,-1},
    { 1, 1, 1},
    { 1,-1, 1},
    {-1, 1, 1},
    {-1,-1, 1}
};

face_t skybox_faces[N_CUBE_FACES] = {
    // front
    { 1, 3, 2, { 0, 0.5 }, { 0, 0 }, { 0.25, 0 }, WHITE },
    { 1, 4, 3, { 0, 0.5 }, { 0.25, 0 }, { 0.25, 0.5 }, WHITE },
    // right
    { 4, 5, 3, { 0, 0.5 }, { 0, 0 }, { 0.25, 0 }, WHITE },
    { 4, 6, 5,  { 0, 0.5 }, { 0.25, 0 }, { 0.25, 0.5 }, WHITE },
    // back
    { 6, 7, 5, { 0, 0.5 }, { 0, 0 }, { 0.25, 0 }, WHITE },
    { 6, 8, 7,  { 0, 0.5 }, { 0.25, 0 }, { 0.25, 0.5 }, WHITE },
    // left
    { 8, 2, 7, { 0, 0.5 }, { 0, 0 }, { 0.25, 0 }, WHITE },
    { 8, 1, 2,  { 0, 0.5 }, { 0.25, 0 }, { 0.25, 0.5 }, WHITE },
    // top
    { 2, 5, 7, { 0, 0.5 }, { 0, 0 }, { 0.25, 0 }, WHITE },
    { 2, 3, 5,  { 0, 0.5 }, { 0.25, 0 }, { 0.25, 0.5 }, WHITE },
    // bottom
    { 6, 1, 8, { 0, 0.5 }, { 0, 0 }, { 0.25, 0 }, WHITE },
    { 6, 4, 1,  { 0, 0.5 }, { 0.25, 0 }, { 0.25, 0.5 }, WHITE }
};

void loadskybox_data(mesh_t &mesh){
    for (int i = 0; i < N_CUBE_VERTICES; i++) {
        vec3_t cube_vertex = cube_vertices[i];
        mesh.vertices.push_back(cube_vertex);
    }
    for (int i = 0; i < N_CUBE_FACES; i++) {
        face_t cube_face = skybox_faces[i];
        mesh.faces.push_back(cube_face);
    }
}