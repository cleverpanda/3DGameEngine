#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>
#include "display.h"
#include "vector.h"
#include "matrix.h"
#include "mesh.h"
#include "Logger.h"
#include "light.h"
#include "camera.h"
//#include "obj_file.h"
#include "texture.h"
#include "triangle.h"
#include <SDL_image.h>
//#include "upng.h"
#include "colors.h"
#include "clipping.h"

bool showWireframe = false;
bool showVertex = false;
bool showFilled = false;
bool showTexture = true;
bool enableBackfaceCulling = true;
bool trackingCamera = false;

///////////////////////////////////////////////////////////////////////////////
// Array of triangles that should be rendered frame by frame
///////////////////////////////////////////////////////////////////////////////
const int MAX_TRIS_TO_RENDER = 10000;
triangle_t triangles_to_render[MAX_TRIS_TO_RENDER];
triangle_t sky_tris[12];
int num_triangles_to_render = 0;
int sky_tris_to_render = 0;
///////////////////////////////////////////////////////////////////////////////
// Global variables for execution status and game loop
///////////////////////////////////////////////////////////////////////////////
bool is_running = false;
int msPrevFrame = 0;
int FPS = 0;
double deltaTime = 0;

mat4_t world_matrix;
mat4_t proj_matrix;
mat4_t view_matrix;

float zNear = 0.1;
float zFar = 1000.0;


///////////////////////////////////////////////////////////////////////////////
// Setup function to initialize variables and game objects
///////////////////////////////////////////////////////////////////////////////
void setup(void) {
    Logger::Log("setup");
    // Allocate the required memory in bytes to hold the color buffer
    color_buffer = (uint32_t*)malloc(sizeof(uint32_t) * window_width * window_height);
    depth_buffer = (float*)malloc(sizeof(float) * window_width * window_height);

    // Creating a SDL texture that is used to display the color buffer
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height
    );
    float aspectx = (float)window_width / (float)window_height;
    float aspecty = (float)window_height / (float)window_width;

    float fovy = deg2rad(60);
    float fovx = atanf(tanf(fovy / 2.0) * aspectx) * 2.0;
    proj_matrix = Matrix_MakeProjection(fovy, aspecty, zNear, zFar);
    
    init_frustum_planes(fovx, fovy, zNear, zFar);

    // Loads the vertex and face values for the mesh data structure
    //load_cube_mesh_data();
    //load_mesh("skybox","sky", vec3_new(10, 10, 5100), vec3_new(0.0, 0.0, 0.0), vec3_new(0, 0, 0));
    load_mesh("runway", vec3_new(1, 1, 1), vec3_new(0, -1.5, +23), vec3_new(0, 0, 0));
    load_mesh("f22", vec3_new(1, 1, 1), vec3_new(0, -1.3, +5), vec3_new(0, -M_PI / 2, 0));
    load_mesh("efa", vec3_new(1, 1, 1), vec3_new(-2, -1.3, +9), vec3_new(0, -M_PI / 2, 0));
    load_mesh("f117", vec3_new(1, 1, 1), vec3_new(+2, -1.3, +9), vec3_new(0, -M_PI / 2, 0));

    //load_obj_file_data("teapot.obj");
    //mesh.rotation.y = 1.5708;
    //mesh.rotation.x = 0.7854;
    //mesh.rotation.y = 0.7854;
    //mesh.rotation.z = 0.7854;
}


///////////////////////////////////////////////////////////////////////////////
// Poll system events and handle keyboard input
///////////////////////////////////////////////////////////////////////////////
void Process_input() {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) { //& gets address
        switch (sdlEvent.type) {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (sdlEvent.key.keysym.sym == SDLK_ESCAPE)
                is_running = false;
            if (sdlEvent.key.keysym.sym == SDLK_g)
                showWireframe = !showWireframe;
            if (sdlEvent.key.keysym.sym == SDLK_v)
                showVertex = !showVertex;
            if (sdlEvent.key.keysym.sym == SDLK_f)
                showFilled = !showFilled;
            if (sdlEvent.key.keysym.sym == SDLK_b)
                enableBackfaceCulling = !enableBackfaceCulling;
            if (sdlEvent.key.keysym.sym == SDLK_t)
                showTexture = !showTexture;
            if (sdlEvent.key.keysym.sym == SDLK_c)
                trackingCamera = !trackingCamera;

            //if (trackingCamera) {
                /*if (sdlEvent.key.keysym.sym == SDLK_w) //move forward
                    camera.vel = vec3_mul(camera.dir, 5.0 * deltaTime);
                    camera.pos = vec3_add(camera.pos, camera.vel);
                if (sdlEvent.key.keysym.sym == SDLK_s) //move back
                    camera.vel = vec3_mul(camera.dir, 5.0 * deltaTime);
                    camera.pos = vec3_sub(camera.pos, camera.vel);
                if (sdlEvent.key.keysym.sym == SDLK_SPACE) //move up
                    camera.pos.y += 3.0 * deltaTime;
                if (sdlEvent.key.keysym.sym == SDLK_LCTRL) //move down
                    camera.pos.y -= 3.0 * deltaTime;
                if (sdlEvent.key.keysym.sym == SDLK_a) //rotate left
                    camera.yaw += 1.0 * deltaTime;
                if (sdlEvent.key.keysym.sym == SDLK_d) //rotate right
                    camera.yaw -= 1.0 * deltaTime;  //remember in radians
                break;*/
            if (sdlEvent.key.keysym.sym == SDLK_UP)
                camera.pos.y += 3.0 * deltaTime;
            if (sdlEvent.key.keysym.sym == SDLK_DOWN)
                camera.pos.y -= 3.0 * deltaTime;
            if (sdlEvent.key.keysym.sym == SDLK_a)
                camera.yaw -= 1.0 * deltaTime;
            if (sdlEvent.key.keysym.sym == SDLK_d)
                camera.yaw += 1.0 * deltaTime;
            if (sdlEvent.key.keysym.sym == SDLK_w) {
                camera.vel = vec3_mul(camera.dir, 5.0 * deltaTime);
                camera.pos = vec3_add(camera.pos, camera.vel);
            }
            if (sdlEvent.key.keysym.sym == SDLK_s) {
                camera.vel = vec3_mul(camera.dir, 5.0 * deltaTime);
                camera.pos = vec3_sub(camera.pos, camera.vel);
            }
            //}
            //break;
            
        }
    }

}

///////////////////////////////////////////////////////////////////////////////
// Update function frame by frame with a fixed time step
///////////////////////////////////////////////////////////////////////////////
//char s[32];
void update(void) {

    deltaTime = (SDL_GetTicks() - msPrevFrame) / 1000.0; //in sec
    msPrevFrame = SDL_GetTicks();
    FPS = (int)1.0f / deltaTime;
    //sprintf(s, "Game - FPS: %i", FPS);
    //SDL_SetWindowTitle(window, s);

    num_triangles_to_render = 0;
    sky_tris_to_render = 0;
    // Change the mesh scale, rotation, and translation values per animation frame
    //mesh.rotation.x += 0.006;
    //mesh.rotation.y += 0.000;
    //mesh.rotation.z += 0.000;
    //mesh.translation.z = 5.0;

    // Change the camera position per animation frame
    mesh_t* mesh = get_mesh(0);

    vec3_t target = { 0, 0, 1 };
    mat4_t camera_yaw_rotation = mat4_make_rotation_y(camera.yaw);
    camera.dir = vec3_from_vec4(mat4_mul_vec4(camera_yaw_rotation, vec4_from_vec3(target)));

    // Offset the camera position in the direction where the camera is pointing at
    target = vec3_add(camera.pos, camera.dir);
    vec3_t up_direction = { 0, 1, 0 };

    // Create the view matrix
    view_matrix = mat4_look_at(camera.pos, target, up_direction);
    mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh->rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh->rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh->rotation.z);
    mat4_t translation_matrix = mat4_make_translation(camera.pos.x, camera.pos.y, camera.pos.z);
    // Loop all triangle faces of our mesh
    int num_faces = mesh->faces.size();

    for (int i = 0; i < num_faces; i++) {
        //if (i != 4) continue;//TODO
        face_t mesh_face = mesh->faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh->vertices[mesh_face.a - 1];
        face_vertices[1] = mesh->vertices[mesh_face.b - 1];
        face_vertices[2] = mesh->vertices[mesh_face.c - 1];

        vec4_t transformed_vertices[3];

        // Loop all three vertices of this current face and apply transformations
        for (int j = 0; j < 3; j++) {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            // Use a matrix to scale our original vertex
            world_matrix = mat4_identity();
            world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
            world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

            // Multiply the world matrix by the original vector
            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

            // Multiply the view matrix by the vector to transform the scene to camera space
            transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);


            // Save transformed vertex in the array of transformed vertices
            transformed_vertices[j] = transformed_vertex;
        }

        // Backface culling test to see if the current face should be projected
        vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]); /*   A   */
        vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]); /*  / \  */
        vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]); /* C---B */

        // Get the vector subtraction of B-A and C-A
        //vec3_t vector_ab = vec3_normalize(vec3_sub(vector_b, vector_a));
        //vec3_t vector_ac = vec3_normalize(vec3_sub(vector_c, vector_a));
        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);
        vec3_normalize(&vector_ab);
        vec3_normalize(&vector_ac);
        // Compute the face normal (using cross product to find perpendicular)
        //vec3_t normal = vec3_normalize(vec3_cross(vector_ab, vector_ac));
        vec3_t normal = vec3_cross(vector_ab, vector_ac);
        vec3_normalize(&normal);
        // Find the vector between vertex A in the triangle and the camera origin
        vec3_t origin = { 0, 0, 0 };
        vec3_t camera_ray = vec3_sub(origin, vector_a);

        // Calculate how aligned the camera ray is with the face normal (using dot product)
        float dot_normal_camera = vec3_dot(normal, camera_ray);

        // Bypass the triangles that are looking away from the camera
        if (enableBackfaceCulling) {
            if (dot_normal_camera < 0) {
                continue;
            }
        }

        //Clipping
        polygon_t polygon = create_polygon_from_triangle(
            vec3_from_vec4(transformed_vertices[0]),
            vec3_from_vec4(transformed_vertices[1]),
            vec3_from_vec4(transformed_vertices[2]),
            mesh_face.a_uv,
            mesh_face.b_uv,
            mesh_face.c_uv
        );

        clip_polygon(&polygon);
        // Break the clipped polygon apart back into individual triangles
        triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
        int num_triangles_after_clipping = 0;

        triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

        // Loops all the assembled triangles after clipping
        for (int t = 0; t < num_triangles_after_clipping; t++) {
            triangle_t triangle_after_clipping = triangles_after_clipping[t];

            vec4_t projected_points[3];

            // Loop all three vertices to perform projection and conversion to screen space
            for (int j = 0; j < 3; j++) {
                // Project the current vertex using a perspective projection matrix
                projected_points[j] = mat4_mul_vec4(proj_matrix, triangle_after_clipping.points[j]);

                // Perform perspective divide
                if (projected_points[j].w != 0) {
                    projected_points[j].x /= projected_points[j].w;
                    projected_points[j].y /= projected_points[j].w;
                    projected_points[j].z /= projected_points[j].w;
                }

                // Flip vertically since the y values of the 3D mesh grow bottom->up and in screen space y values grow top->down
                projected_points[j].y *= -1;

                // Scale into the view
                projected_points[j].x *= (window_width / 2.0);
                projected_points[j].y *= (window_height / 2.0);

                // Translate the projected points to the middle of the screen
                projected_points[j].x += (window_width / 2.0);
                projected_points[j].y += (window_height / 2.0);
            }

            // Create the final projected triangle that will be rendered in screen space
            triangle_t triangle_to_render = {
                {
                    { projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w },
                    { projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w },
                    { projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w },
                },
                {
                    { triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v },
                    { triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v },
                    { triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v }
                },
                WHITE,
                mesh->texture
            };

            sky_tris[sky_tris_to_render++] = triangle_to_render;
        }
    }

    for(int mesh_index = 1; mesh_index < get_num_meshes(); mesh_index++) {
        mesh_t* mesh = get_mesh(mesh_index);
        //Handle switching camera modes
        //create matrix looking at target point
        //if (trackingCamera) {
        //    target = { 0.0, 0.0, 4.0 };
        //    view_matrix = mat4_look_at(camera.pos, target, up_direction);
        //}
        //use RPG style camera
        //else {
        // Initialize the target looking at the positive z-axis
        vec3_t target = { 0, 0, 1 };
        mat4_t camera_yaw_rotation = mat4_make_rotation_y(camera.yaw);
        camera.dir = vec3_from_vec4(mat4_mul_vec4(camera_yaw_rotation, vec4_from_vec3(target)));

        // Offset the camera position in the direction where the camera is pointing at
        target = vec3_add(camera.pos, camera.dir);
        vec3_t up_direction = { 0, 1, 0 };

        // Create the view matrix
        view_matrix = mat4_look_at(camera.pos, target, up_direction);



        // Create scale, rotation, and translation matrices that will be used to multiply the mesh vertices
        mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
        mat4_t translation_matrix = mat4_make_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);
        mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh->rotation.x);
        mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh->rotation.y);
        mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh->rotation.z);

        // Loop all triangle faces of our mesh
        int num_faces = mesh->faces.size();

        for (int i = 0; i < num_faces; i++) {
            //if (i != 4) continue;//TODO
            face_t mesh_face = mesh->faces[i];

            vec3_t face_vertices[3];
            face_vertices[0] = mesh->vertices[mesh_face.a - 1];
            face_vertices[1] = mesh->vertices[mesh_face.b - 1];
            face_vertices[2] = mesh->vertices[mesh_face.c - 1];

            vec4_t transformed_vertices[3];

            // Loop all three vertices of this current face and apply transformations
            for (int j = 0; j < 3; j++) {
                vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

                // Use a matrix to scale our original vertex
                world_matrix = mat4_identity();
                world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
                world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
                world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
                world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
                world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

                // Multiply the world matrix by the original vector
                transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

                // Multiply the view matrix by the vector to transform the scene to camera space
                transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);


                // Save transformed vertex in the array of transformed vertices
                transformed_vertices[j] = transformed_vertex;
            }

            // Backface culling test to see if the current face should be projected
            vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]); /*   A   */
            vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]); /*  / \  */
            vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]); /* C---B */

            // Get the vector subtraction of B-A and C-A
            //vec3_t vector_ab = vec3_normalize(vec3_sub(vector_b, vector_a));
            //vec3_t vector_ac = vec3_normalize(vec3_sub(vector_c, vector_a));
            vec3_t vector_ab = vec3_sub(vector_b, vector_a);
            vec3_t vector_ac = vec3_sub(vector_c, vector_a);
            vec3_normalize(&vector_ab);
            vec3_normalize(&vector_ac);
            // Compute the face normal (using cross product to find perpendicular)
            //vec3_t normal = vec3_normalize(vec3_cross(vector_ab, vector_ac));
            vec3_t normal = vec3_cross(vector_ab, vector_ac);
            vec3_normalize(&normal);
            // Find the vector between vertex A in the triangle and the camera origin
            vec3_t origin = { 0, 0, 0 };
            vec3_t camera_ray = vec3_sub(origin, vector_a);

            // Calculate how aligned the camera ray is with the face normal (using dot product)
            float dot_normal_camera = vec3_dot(normal, camera_ray);

            // Bypass the triangles that are looking away from the camera
            if (enableBackfaceCulling) {
                if (dot_normal_camera < 0) {
                    continue;
                }
            }

            //Clipping
            polygon_t polygon = create_polygon_from_triangle(
                vec3_from_vec4(transformed_vertices[0]),
                vec3_from_vec4(transformed_vertices[1]),
                vec3_from_vec4(transformed_vertices[2]),
                mesh_face.a_uv,
                mesh_face.b_uv,
                mesh_face.c_uv
            );

            clip_polygon(&polygon);
            // Break the clipped polygon apart back into individual triangles
            triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
            int num_triangles_after_clipping = 0;

            triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

            // Loops all the assembled triangles after clipping
            for (int t = 0; t < num_triangles_after_clipping; t++) {
                triangle_t triangle_after_clipping = triangles_after_clipping[t];

                vec4_t projected_points[3];

                // Loop all three vertices to perform projection and conversion to screen space
                for (int j = 0; j < 3; j++) {
                    // Project the current vertex using a perspective projection matrix
                    projected_points[j] = mat4_mul_vec4(proj_matrix, triangle_after_clipping.points[j]);

                    // Perform perspective divide
                    if (projected_points[j].w != 0) {
                        projected_points[j].x /= projected_points[j].w;
                        projected_points[j].y /= projected_points[j].w;
                        projected_points[j].z /= projected_points[j].w;
                    }

                    // Flip vertically since the y values of the 3D mesh grow bottom->up and in screen space y values grow top->down
                    projected_points[j].y *= -1;

                    // Scale into the view
                    projected_points[j].x *= (window_width / 2.0);
                    projected_points[j].y *= (window_height / 2.0);

                    // Translate the projected points to the middle of the screen
                    projected_points[j].x += (window_width / 2.0);
                    projected_points[j].y += (window_height / 2.0);
                }

                // Calculate the shade intensity based on how aliged is the normal with the flipped light direction ray
                // Calculate the triangle color based on the light angle
                float luminance = -vec3_dot(normal, sun.direction);
                uint32_t tri_color = light_apply_intensity(mesh_face.color, luminance);

                // Create the final projected triangle that will be rendered in screen space
                triangle_t triangle_to_render = {
                    {
                        { projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w },
                        { projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w },
                        { projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w },
                    },
                    {
                        { triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v },
                        { triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v },
                        { triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v }
                    },
                    tri_color,
                    mesh->texture
                };

                // Save the projected triangle in the array of triangles to render
                if (num_triangles_to_render < MAX_TRIS_TO_RENDER) {
                    triangles_to_render[num_triangles_to_render++] = triangle_to_render;
                }
            }
        }
    }
}
void draw_skybox() {
    for (int i = 0; i < 12; i++) {
        triangle_t triangle = sky_tris[i];
        draw_textured_triangle(
            triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v, // vertex A
            triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v, // vertex B
            triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v, // vertex C
            triangle.texture
        );
    }
}

///////////////////////////////////////////////////////////////////////////////
// Render function to draw objects on the display
///////////////////////////////////////////////////////////////////////////////
void render(void) {
    SDL_RenderClear(renderer);

    draw_grid();
    draw_skybox();
    // Loop all projected triangles and render them
    for (int i = 0; i < num_triangles_to_render; i++) {
        triangle_t triangle = triangles_to_render[i];

        // Draw filled triangle
        if (showFilled) {
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, // vertex A
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, // vertex B
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, // vertex C
                triangle.color
            );
        }
        if (showTexture) {
            draw_textured_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v, // vertex A
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v, // vertex B
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v, // vertex C
                triangle.texture
            );
        }
        
        // Draw triangle wireframe
        if (showWireframe) {
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y, // vertex A
                triangle.points[1].x, triangle.points[1].y, // vertex B
                triangle.points[2].x, triangle.points[2].y, // vertex C
                0xFFFFFFFF
            );
        }

        // Draw triangle vertex points
        if (showVertex) {
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFFFF00); // vertex A
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFFFF00); // vertex B
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFFFF00); // vertex C
        }
    }

    // Clear the array of triangles to render every frame loop
    //array_free(triangles_to_render);

    render_color_buffer();
    //alpha|blue|green|red
    clear_color_buffer(0xFF000000); //0x008080FF
    clear_depth_buffer();

    SDL_RenderPresent(renderer);
}

void free_meshes(void) {
    for (int mesh_index = 0; mesh_index < get_num_meshes(); mesh_index++) {
        mesh_t* mesh = get_mesh(mesh_index);
        SDL_FreeSurface(mesh->texture);
    }
}

///////////////////////////////////////////////////////////////////////////////
// Free the memory that was dynamically allocated by the program
///////////////////////////////////////////////////////////////////////////////
void free_resources(void) {
    Logger::Log(std::to_string(FPS));
    free(color_buffer);
    free(depth_buffer);
    free_meshes();
    //free(&mesh.faces);
    //free(&mesh.vertices);
}



///////////////////////////////////////////////////////////////////////////////
// Main function
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
    is_running = initialize_window();

    setup();

    while (is_running) {
        Process_input();
        update();
        render();
    }

    destroy_window();
    free_resources();

    return 0;
}
