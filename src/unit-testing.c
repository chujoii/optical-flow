/** \file
unit-testing.c --- Unit-testing block-matching f-or optical-flow algo-rithm

Copyright (C) 2022 Roman V. Prikhodchenko

Author: Roman V. Prikhodchenko <chujoii@gmail.com>


    This file is part of optical-flow.

    optical-flow is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    optical-flow is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with optical-flow.  If not, see <http://www.gnu.org/licenses/>.



Keywords: projective geometry matrix manipulation homogeneous coordinates

Usage:

History:

Code:
*/

#include <stdio.h>
#include <string.h> // memset

#include "const.h"
#include "gui.h"
#include "block-matching.h"

//#define DEBUG

#define IMG_SIZE 12
#define BLOCK_SIZE_TEST 4
#define MAX_SHIFT_TEST 2
#define FPS 25

// fixme: global variables
struct imgRawImage* raw_image;
struct imgRawImage* gui_image;
struct imgRawImage* old_image;
int verbose = VERBOSE_NO;
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

unsigned char image_a0 [] = {
//      1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  1
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  2
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  3
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  4
	0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0,   //  5
	0, 0, 0, 0, 8, 0, 8, 0, 0, 0, 0, 0,   //  6
	0, 0, 0, 0, 8, 8, 8, 0, 0, 0, 0, 0,   //  7
	0, 0, 0, 0, 8, 0, 8, 0, 0, 0, 0, 0,   //  8
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  9
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 10
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 11
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // 12

unsigned char image_a1 [] = {
//      1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  1
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  2
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  3
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  4
	0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0,   //  5
	0, 0, 0, 0, 0, 7, 0, 9, 0, 0, 0, 0,   //  6
	0, 0, 0, 0, 0, 7, 9, 9, 0, 0, 0, 0,   //  7
	0, 0, 0, 0, 3, 7, 0, 9, 0, 0, 0, 0,   //  8
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   //  9
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 10
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,   // 11
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};  // 12

unsigned char image_empty [IMG_SIZE*IMG_SIZE];



int main ()
{
	COORD_2D block = {0, 0};
	COORD_2D shift = {0, 0};


	raw_image = (struct imgRawImage*)malloc(sizeof(struct imgRawImage));
	raw_image->width = IMG_SIZE;
	raw_image->height = IMG_SIZE;
	raw_image->numComponents = 1;
	raw_image->dwBufferBytes = raw_image->width * raw_image->height * raw_image->numComponents;
	raw_image->lpData = image_empty;

	old_image = (struct imgRawImage*)malloc(sizeof(struct imgRawImage));
	old_image->width = IMG_SIZE;
	old_image->height = IMG_SIZE;
	old_image->numComponents = 1;
	old_image->dwBufferBytes = old_image->width * old_image->height * old_image->numComponents;
	old_image->lpData = image_empty;

	gui_image = (struct imgRawImage*)malloc(sizeof(struct imgRawImage));
	gui_image->width = IMG_SIZE;
	gui_image->height = IMG_SIZE;
	gui_image->numComponents = 1;
	gui_image->dwBufferBytes = gui_image->width * gui_image->height * gui_image->numComponents;
	gui_image->lpData = (unsigned char*)malloc(sizeof(unsigned char)*gui_image->dwBufferBytes);

	memset(image_empty, 0, IMG_SIZE*IMG_SIZE * sizeof(unsigned char));

	OPTICAL_FLOW flow;
	init_block_matching (raw_image->width, raw_image->height, BLOCK_SIZE_TEST, MAX_SHIFT_TEST, NANOSECONDS_IN_SECOND / FPS, &flow);



	memset(gui_image->lpData, 0, IMG_SIZE*IMG_SIZE * sizeof(unsigned char));
	block.x = 4; block.y = 4;
	shift.x = 5; shift.y = 6;
	// uncomment "#define DEBUG" in top of file
	diff_block (old_image, raw_image, gui_image, block, shift, BLOCK_SIZE_TEST);
	print_image (gui_image);





	// comment "#define DEBUG" in top of file
	raw_image->lpData = image_a1;
	old_image->lpData = image_a0;
	COORD_2D best_shift = find_block_correlation (old_image, raw_image, gui_image, block, MAX_SHIFT_TEST, BLOCK_SIZE_TEST);

	printf("\n\n.A\n");
	print_image (old_image);

	printf("\n\nA\n");
	print_image (raw_image);

	printf("best shift = [%ld %ld]\n", best_shift.x, best_shift.y);
	


	block_matching_full_images (old_image, raw_image, gui_image, MAX_SHIFT_TEST, BLOCK_SIZE_TEST);
	print_image (gui_image);

	free(gui_image->lpData);
	free_block_matching (&flow);
	return 0;
}
