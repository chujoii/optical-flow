/** \file
block-matching.c --- Base algorithm of optical-flow

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

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

#include "const.h"
#include "image.h"
#include "util.h"
#include "block-matching.h"
#include "block-matching-type.h"



long long int coord_to_raw_flow(OPTICAL_FLOW * flow, COORD_2DU coord)
{
	if (coord.x >= flow->width ||
	    coord.y >= flow->height) return -1;
	long long int raw_chunk = coord.y * flow->width + coord.x;
	if (raw_chunk > (long long int)flow->array_size) return -1;
	return raw_chunk;
}



COORD_2DU raw_flow_to_coord(OPTICAL_FLOW* flow, unsigned long int r)
{
	COORD_2DU c;
	c.y = r / flow->width;
	c.x = (r - c.y * flow->width);
	return c;
}



int init_block_matching (int image_width, int image_height, int block_size, int max_shift, long int nspf, OPTICAL_FLOW* flow)
{
	flow->block_size_in_pixel = block_size;
	flow->max_shift = max_shift;
	flow->nspf = nspf;

	flow->width = get_block_numbers (image_width,  block_size);
	flow->height = get_block_numbers (image_height, block_size);
	flow->array_size = flow->width * flow->height;

	flow->array = (BLK*) malloc(sizeof(BLK) * flow->array_size);

	for (unsigned long int j = 0; j < flow->height; j++) {
		for (unsigned long int i = 0; i < flow->width; i++) {
			COORD_2DU coord = {.x=i, .y=j};
			unsigned long int index = coord_to_raw_flow(flow, coord);
			flow->array[index].last_update = 0;
			flow->array[index].shift.x = 0;
			flow->array[index].shift.y = 0;
		}
	}
	return 0;
}



void free_block_matching (OPTICAL_FLOW* flow)
{
	free(flow->array);
}



void print_image (struct imgRawImage* image)
{
	for (unsigned int j = 0; j < image->height; j++) {
		for (unsigned int i = 0; i < image->width; i++) {
			int c = image->lpData[coord_to_raw_chunk(image, (COORD_2DU){i, j})];
			if (c == 0) {
				printf (" .");
			} else {
				printf("%2d", c);
			}
		}
		printf("\n");
	}
}


int get_block_numbers (int image_size, int block_size)
{
	return image_size / block_size + ((image_size % block_size > 0) ? 1 : 0);
}



/**
   block_size = 8 (for example)
   coord of block = coord of left top element (0, 0)


   . 0 1 2 3 4 5 6 7
   0 . . . . . . . .
   1 . . . . . . . .
   2 . . . . . . . .
   3 . . . . . . . .
   4 . . . . . . . .
   5 . . . . . . . .
   6 . . . . . . . .
   7 . . . . . . . .




   real pixel contains some components: RGB or RGBI (Infrared):

   .  0   1   2   3   4   5   6   7
   0 ... ... ... ... ... ... ... ...
   1 ... ... ... ... ... ... ... ...
   2 ... . . .



   old = block coord in old image
   new = block coord in raw image
*/
double diff_block (struct imgRawImage* old_image, struct imgRawImage* new_image, struct imgRawImage* gui_image,
		   COORD_2D block, COORD_2D shift, int block_size)
{
	(void)*gui_image; // suppress "unused parameter" warnings

	unsigned long int sum = 0;
	unsigned long int counter = 0;

	// fixme: add simultaneous rotation and translation
	COORD_2D coord_2d_old;
	COORD_2D coord_2d_new;
	long long int coord_raw_old;
	long long int coord_raw_new;
	int nx, ny; // index for new image

	for (ny = 0; ny < block_size; ny++) {
		coord_2d_old.y = block.y + ny;
		coord_2d_new.y = block.y + ny + shift.y;
		for (nx = 0; nx < block_size; nx++) {
			coord_2d_old.x = block.x + nx;
			coord_2d_new.x = block.x + nx + shift.x;

			if (coord_2d_old.x >= 0 && coord_2d_old.y >= 0 &&
			    coord_2d_new.x >= 0 && coord_2d_new.y >= 0) {
				COORD_2DU coord_2du_old = {coord_2d_old.x, coord_2d_old.y};
				COORD_2DU coord_2du_new = {coord_2d_new.x, coord_2d_new.y};
				coord_raw_old = coord_to_raw_chunk(old_image, coord_2du_old);
				coord_raw_new = coord_to_raw_chunk(new_image, coord_2du_new);

				if (coord_raw_old >= 0 && coord_raw_new >= 0) {
					for (unsigned int color = 0; color < new_image->numComponents; color++) {
#ifdef DEBUG
						gui_image->lpData[coord_raw_old + color] += 20;
						gui_image->lpData[coord_raw_new + color] += 1;
#endif
						sum += abs( (int)(old_image->lpData[coord_raw_old + color]) -
							    (int)(new_image->lpData[coord_raw_new + color]));
						counter++;
					}
				}
			}
		}
	}

	if (counter == 0) return 0;
	return (double)sum/(double)counter;
}



static int cmp_double(const void * a, const void * b)
{
	return (((HISTOGRAM_STORAGE *)a)->diff > ((HISTOGRAM_STORAGE *)b)->diff) ? -1 : 1;
}



/**
   Find block correlation with all possible shifts
*/
COORD_2D find_block_correlation (struct imgRawImage* old_image, struct imgRawImage* new_image, struct imgRawImage* gui_image, COORD_2D block, int max_shift, int block_size)
{
	double result;
	COORD_2D shift = {0, 0};

	COORD_2D best_shift = shift;
	double min_result = diff_block (old_image, new_image, gui_image, block, shift, block_size);
	double max_result = min_result;

	if (min_result < EPSILON) return best_shift;

	HISTOGRAM_STORAGE histogram[SQUARE(max_shift * 2 + 1)];
	int counter = 0;

	for (int j = -max_shift; j <= max_shift; j++) {
		for (int i = -max_shift; i <= max_shift; i++) {
			shift.x = i; shift.y = j;
			result = diff_block (old_image, new_image, gui_image, block, shift, block_size);

			histogram[counter].diff = result;
			histogram[counter].shift.x = i;
			histogram[counter].shift.y = j;
			counter++;
		}
	}

	qsort(histogram, counter, sizeof(HISTOGRAM_STORAGE), cmp_double); // h[0] = max; h[counter-1] = min
	double median = histogram[counter/2].diff;
	min_result = histogram[counter - 1].diff;
	max_result = histogram[0].diff;
	best_shift = histogram[counter - 1].shift;

	if (median - min_result < THRESHOLD) return (COORD_2D) {0, 0};
	//if ((median - min_result) / (max_result - median)  < THRESHOLD) return (COORD_2D) {0, 0};

	int i = counter - 1;
	double best_distance = sqrt(2*SQUARE(max_shift));
	double distance;

	// some shift variants --- equal by "diff" value, so find shift variant with smallest distance to center
	while (i > 0 && histogram[i].diff - min_result < HISTOGRAM_EPSILON) {
		distance = sqrt(SQUARE(histogram[i].shift.x) + SQUARE(histogram[i].shift.y));
		if (distance < best_distance) {
			best_distance = distance;
			best_shift = histogram[i].shift;
		}

		i--;
	}

	return best_shift;
}



void block_matching_full_images (struct imgRawImage* old_image, struct imgRawImage* new_image, struct imgRawImage* gui_image,
				 int max_shift, int block_size)
{
	int horizontal_blocks_num = get_block_numbers (new_image->width,  block_size);
	int vertical_blocks_num   = get_block_numbers (new_image->height, block_size);

	COORD_2D coord_shift;
	COORD_2D block;
	COORD_2DU pixel;
	long long int coord_raw;

	RGB_COLOR color_shift;

	for (int j=0; j < vertical_blocks_num; j++) {
		block.y = j * block_size;
		for (int i=0; i < horizontal_blocks_num; i++) {
			block.x = i * block_size;
			coord_shift = find_block_correlation (old_image, new_image, gui_image, block, max_shift, block_size);

			for(pixel.y = block.y; pixel.y < (unsigned long int)(block.y + block_size); pixel.y++) {
				for(pixel.x = block.x; pixel.x < (unsigned long int)(block.x + block_size); pixel.x++) {
					coord_raw = coord_to_raw_chunk(gui_image, pixel);
					if (coord_raw > 0) {
						RGB_COLOR source_color = {
							.r = new_image->lpData[coord_raw + R],
							.g = new_image->lpData[coord_raw + G],
							.b = new_image->lpData[coord_raw + B]};
						color_shift = shift_to_color (source_color, coord_shift, max_shift);
						gui_image->lpData[coord_raw + R] = color_shift.r;
						gui_image->lpData[coord_raw + G] = color_shift.g;
						gui_image->lpData[coord_raw + B] = color_shift.b;
					}
				}
			}
			
		}
	}
}



void block_matching_optimized_images (struct imgRawImage* old_image, struct imgRawImage* new_image, struct imgRawImage* gui_image,
				      OPTICAL_FLOW* flow)
{
	int horizontal_blocks_num = flow->width;
	int vertical_blocks_num   = flow->height;

	COORD_2D coord_shift;
	COORD_2D block;

	struct timespec ts_start;
        struct timespec ts_current;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);


	// find in previous success blocks
	for (int j=0; j < vertical_blocks_num; j++) {
		block.y = j * flow->block_size_in_pixel;
		for (int i=0; i < horizontal_blocks_num; i++) {
			block.x = i * flow->block_size_in_pixel;
			int raw_flow_coord = coord_to_raw_flow(flow, (COORD_2DU) {.x=i, .y=j});
			if (labs(flow->array[raw_flow_coord].shift.x) + labs(flow->array[raw_flow_coord].shift.y) > 0) {
				coord_shift = find_block_correlation (old_image, new_image, gui_image, block, flow->max_shift, flow->block_size_in_pixel);
				flow->array[raw_flow_coord].shift = coord_shift;
				flow->array[raw_flow_coord].last_update = 0;
			}
		}
	}


	// process random block
	int time_duration;
	do {
		int i = rnd(0, horizontal_blocks_num - 1);
		int j = rnd(0, vertical_blocks_num - 1);
		int raw_flow_coord = coord_to_raw_flow(flow, (COORD_2DU) {.x=i, .y=j});
		if (flow->array[raw_flow_coord].last_update != 0) {
			block.x = i * flow->block_size_in_pixel;
			block.y = j * flow->block_size_in_pixel;

			coord_shift = find_block_correlation (old_image, new_image, gui_image, block, flow->max_shift, flow->block_size_in_pixel);
			flow->array[raw_flow_coord].shift = coord_shift;
			flow->array[raw_flow_coord].last_update = 0;
		}

		clock_gettime(CLOCK_MONOTONIC, &ts_current);
		if (ts_current.tv_sec - ts_start.tv_sec == 0) {
			time_duration = ts_current.tv_nsec - ts_start.tv_nsec;
		} else { // fixme: if more than one seconds, then need add seconds diff
			time_duration = ts_start.tv_nsec - ts_current.tv_nsec;
		}
	} while (time_duration < flow->nspf);


	for (unsigned long int i = 0; i < flow->array_size; i++) {
		flow->array[i].last_update += 1;
	}

	colorize(new_image, gui_image, flow);
}



void colorize (struct imgRawImage* new_image, struct imgRawImage* gui_image, OPTICAL_FLOW* flow)
{
	int horizontal_blocks_num = flow->width;
	int vertical_blocks_num   = flow->height;

	COORD_2D coord_shift;
	COORD_2D block;
	COORD_2DU pixel;
	long long int coord_raw;

	RGB_COLOR color_shift;
		
	coord_shift = (COORD_2D) {.x=0, .y=0};
	for (int j=0; j < vertical_blocks_num; j++) {
		block.y = j * flow->block_size_in_pixel;
		for (int i=0; i < horizontal_blocks_num; i++) {
			block.x = i * flow->block_size_in_pixel;
			int raw_flow_coord = coord_to_raw_flow(flow, (COORD_2DU) {.x=i, .y=j});
			coord_shift = flow->array[raw_flow_coord].shift;
			for(pixel.y = block.y; pixel.y < (unsigned long int)(block.y + flow->block_size_in_pixel); pixel.y++) {
				for(pixel.x = block.x; pixel.x < (unsigned long int)(block.x + flow->block_size_in_pixel); pixel.x++) {
					coord_raw = coord_to_raw_chunk(gui_image, pixel);
					if (coord_raw > 0) {
						RGB_COLOR source_color = {
							.r = new_image->lpData[coord_raw + R],
							.g = new_image->lpData[coord_raw + G],
							.b = new_image->lpData[coord_raw + B]};
						color_shift = shift_to_color (source_color, coord_shift, flow->max_shift);
						gui_image->lpData[coord_raw + R] = color_shift.r;
						gui_image->lpData[coord_raw + G] = color_shift.g;
						gui_image->lpData[coord_raw + B] = color_shift.b;
					}
				}
			}
		}
	}
}



RGB_COLOR shift_to_color (RGB_COLOR source_color, COORD_2D shift, int max_shift)
{
	double monochrome = (0.2125 * source_color.r) + (0.7154 * source_color.g) + (0.0721 * source_color.b);

	// HSL (for hue, saturation, lightness) and HSV (for hue, saturation, value; also known as HSB, for hue, saturation, brightness)
	// convert HSL to RGB
	// Given a color with hue H [0, 360], saturation S [0, 1], and lightness L [0, 1]
	double hue = convert_radian_to_degree(angle_modulo(atan2(shift.y, shift.x)));
	double saturation = sqrt((double)SQUARE(shift.x) + (double)SQUARE(shift.y)) / (double) max_shift;
	double lightness = float_constrain(monochrome / 255.0, 0.0, 1.0);

	double c = (1.0 - fabs(2.0*lightness - 1.0)) * saturation; // chroma
	double h = hue / 60.0; // neighbour
	double x = c * (1.0 - fabs(fmod(h, 2.0) - 1.0));
	double m = lightness - c/2.0;

	double cr = c;
	double cg = x;
	double cb = 0;
	if (0.0 <= h && h < 1.0) {cr = c; cg = x; cb = 0;}
	if (1.0 <= h && h < 2.0) {cr = x; cg = c; cb = 0;}
	if (2.0 <= h && h < 3.0) {cr = 0; cg = c; cb = x;}
	if (3.0 <= h && h < 4.0) {cr = 0; cg = x; cb = c;}
	if (4.0 <= h && h < 5.0) {cr = x; cg = 0; cb = c;}
	if (5.0 <= h           ) {cr = c; cg = 0; cb = x;} // 5.0 <= h && h < 6.0

	RGB_COLOR rgb = {
		.r = (cr + m) * 255,
		.g = (cg + m) * 255,
		.b = (cb + m) * 255,
	};
		
	return rgb;
}
