/** \file
   gui.h --- header for gui.c

   Copyright (C) 2021 Roman V. Prikhodchenko

   Author: Roman V. Prikhodchenko <chujoii@gmail.com>
*/

// include guard
#ifndef GUI_H
#define GUI_H

// for glGenBuffers
#define GL_GLEXT_PROTOTYPES

//#include <GL/gl.h>
//#include <GL/glext.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <libavutil/frame.h>

#include "image.h"
#include "gui-type.h"

#define MAX_FNAME_LEN 128

#define NUM_OF_SHADER 2
#define SHADER_V 0
#define SHADER_W 1

int init_gui (unsigned int src_width, unsigned int src_height);
unsigned int init_shader_video(void);
void init_shader_widget(unsigned long polylini_size, float *polyline);
void render_loop (struct imgRawImage* gui_image, unsigned int video_texture);
void deallocate_resources (void);
void * start_gui_process(void *arguments);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
int processInput(GLFWwindow *window);
void myInit (void);
void display (void);
void draw_crosshair (struct imgRawImage* image);
void draw_graph (struct imgRawImage* draw_image, int len_data, float * data, int shift_x, int shift_y, int size_x, int size_y, int color);
int convert_i_c (int ai, int shift, float zoom);
int convert_c_i (int ac, int shift, float zoom);
float get_zoom_ratio (int canvas_width, int canvas_height, int image_width, int image_height);

#endif /* GUI_H */
