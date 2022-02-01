#include <stdio.h>
#include <string.h>
#include <sdl_image.h>
#include "mesh.h"
#include "colors.h"
//#include "upng.h"
#include "Logger.h"


#define MAX_NUM_MESHES 100
static mesh_t meshes[MAX_NUM_MESHES];
static int mesh_count = 0;


//void load_textured_obj(std::string name) {
 //   load_png_texture_data(name + ".png");
 //   load_obj_file_data(name);
//}

void load_mesh(std::string name,vec3_t scale, vec3_t translation,  vec3_t rotation) {
    load_mesh(name, name, scale, translation, rotation);
}
void load_mesh(std::string modelname, std::string pngname,
    vec3_t scale, vec3_t translation,vec3_t rotation) {
    load_mesh_obj_data(&meshes[mesh_count], modelname);
    load_mesh_png_data(&meshes[mesh_count], pngname);
    meshes[mesh_count].scale = scale;
    meshes[mesh_count].translation = translation;
    meshes[mesh_count].rotation = rotation;
    mesh_count++;
}

void load_mesh_obj_data(mesh_t* mesh, std::string filename) {
    FILE* file;
    errno_t eCode;
    std::string fullStr = "./assets/obj/" + filename+".obj";
    eCode = fopen_s(&file, fullStr.c_str(), "r");
    char line[1024];
    std::vector<tex2_t> texcoords;

    if (file == NULL) {
       Logger::Err("Couldn't open file");
        return;
    }

    while (fgets(line, 1024, file) != NULL) {
        // Vertex information
        if (strncmp(line, "v ", 2) == 0) {
            vec3_t vertex;
            sscanf_s(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
            mesh->vertices.push_back(vertex);
        }
        // Texture coordinate information
        if (strncmp(line, "vt ", 3) == 0) {
            tex2_t texcoord;
            sscanf(line, "vt %f %f", &texcoord.u, &texcoord.v);
            texcoords.push_back(texcoord);
        }
        // Face information
        if (strncmp(line, "f ", 2) == 0) {
            int vertex_indices[3];
            int texture_indices[3];
            int normal_indices[3];
            sscanf_s(
                line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vertex_indices[0], &texture_indices[0], &normal_indices[0],
                &vertex_indices[1], &texture_indices[1], &normal_indices[1],
                &vertex_indices[2], &texture_indices[2], &normal_indices[2]
            );
            face_t theface = {
                vertex_indices[0],
                vertex_indices[1],
                vertex_indices[2],
                texcoords[texture_indices[0] - 1],
                texcoords[texture_indices[1] - 1],
                texcoords[texture_indices[2] - 1],
                WHITE
            };
            mesh->faces.push_back(theface);
        }
    }
}

void load_mesh_png_data(mesh_t* mesh, std::string filename){
    std::string fullStr = "./assets/obj/" + filename + ".png";

    SDL_Surface* image;
    image = IMG_Load(fullStr.c_str());
    if (image) {
        mesh->texture = image;
        // handle error
    }
    printf("IMG_Load: %s\n", IMG_GetError());


    //upng_t* png_image = upng_new_from_file(fullStr.c_str());
    //if (png_image != NULL) {
    //    upng_decode(png_image);
     //   if (upng_get_error(png_image) == UPNG_EOK) {
     //       mesh->texture = png_image;
            //mesh_texture = (uint32_t*)upng_get_buffer(png_texture);
            //texture_width = upng_get_width(png_texture);
            //texture_height = upng_get_height(png_texture);
     //   }
    //}
}

int get_num_meshes() {
    return mesh_count;
}

mesh_t* get_mesh(int index) {
    return &meshes[index];
}
