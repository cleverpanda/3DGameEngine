#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <SDL.h>


extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern uint32_t* color_buffer;
extern float* depth_buffer;
extern SDL_Texture* color_buffer_texture;
extern int window_width;
extern int window_height;

bool initialize_window();
void draw_grid();
void draw_pixel(int x, int y, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void render_color_buffer();
void clear_color_buffer(uint32_t color);
void destroy_window();
void clear_depth_buffer();