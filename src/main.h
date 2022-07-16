/** \file
   main.h --- header for main.c

   Copyright (C) 2021 Roman V. Prikhodchenko

   Author: Roman V. Prikhodchenko <chujoii@gmail.com>
*/

// include guard
#ifndef MAIN_H
#define MAIN_H

#define NUM_THREADS 5

// fixme: global variables
struct imgRawImage* raw_image;
struct imgRawImage* gui_image;
struct imgRawImage* old_image;
int verbose = VERBOSE_NO;
int hide_static_block = false;
GLFWwindow* window;
unsigned int shaderProgram_video;
unsigned int shaderProgram_widget;
unsigned int VBOs[NUM_OF_SHADER]; // vertex buffer objects
unsigned int VAOs[NUM_OF_SHADER]; // vertex array objects
unsigned int EBOs[NUM_OF_SHADER]; // element buffer objects

int global_step; // in pixels
int status_coord_of_robot = false;
int status_coord_of_fire  = false;
int mouse_state = NO_POINT_SET;
int glob_filter_type = MEDIAN;
int verbose_type = VERBT_FILTER;
int escape_status = false;
int glob_zero_point_fix = 0;
float glob_zoom_ratio = 1.0;
float glob_canvas_shift_y = 0.0;
unsigned int video_texture = 0;

#endif /* MAIN_H */
