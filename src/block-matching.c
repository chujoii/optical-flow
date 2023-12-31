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
#include <pthread.h>
#include <stdatomic.h>

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



int init_block_matching (int image_width, int image_height, int block_size, int max_shift_global, int max_shift_local, double epsilon, double histogram_epsilon, double threshold, int min_neighbours, int long_time_without_update, int painted_by_neighbor, OPTICAL_FLOW* flow)
{
	flow->block_size_in_pixel = block_size;
	flow->max_shift_global = max_shift_global;
	flow->max_shift_local = max_shift_local;
	flow->epsilon = epsilon;
	flow->histogram_epsilon = histogram_epsilon;
	flow->threshold = threshold;
	flow->min_neighbours = min_neighbours;
	flow->long_time_without_update = long_time_without_update;
	flow->painted_by_neighbor = painted_by_neighbor;

	flow->width = get_block_numbers (image_width,  block_size);
	flow->height = get_block_numbers (image_height, block_size);
	flow->array_size = flow->width * flow->height;

	flow->array = (BLK*) malloc(sizeof(BLK) * flow->array_size);
	for (unsigned long int i = 0; i < flow->array_size; i++) {
		flow->array[i].last_update = OPTICAL_FLOW_JUST_UPDATED;
		flow->array[i].shift.x = 0;
		flow->array[i].shift.y = 0;
	}

	atomic_init(&flow->semaphore_optical_flow, true);

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
			long long int index = coord_to_raw_chunk(image, (COORD_2DU){i, j});
			if (index >= 0) {
				int c = image->lpData[index];
				if (c == 0) {
					printf (" .");
				} else {
					printf("%2d", c);
				}
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

		nx = 0;
		coord_2d_old.x = block.x + nx;
		coord_2d_new.x = block.x + nx + shift.x;

		COORD_2DU coord_2du_old = {coord_2d_old.x, coord_2d_old.y};
		COORD_2DU coord_2du_new = {coord_2d_new.x, coord_2d_new.y};
		coord_raw_old = coord_to_raw_chunk(old_image, coord_2du_old);
		coord_raw_new = coord_to_raw_chunk(new_image, coord_2du_new);
		if (coord_raw_old >= 0 && coord_raw_new >= 0) {


			for (nx = 0; nx < block_size; nx++) {
				coord_raw_old += nx * old_image->numComponents;
				coord_raw_new += nx * old_image->numComponents;
				if (coord_raw_old <= (long long int)old_image->dwBufferBytes &&
				    coord_raw_new <= (long long int)new_image->dwBufferBytes) {
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
COORD_2D find_block_correlation (struct imgRawImage* old_image, struct imgRawImage* new_image, struct imgRawImage* gui_image,
				 COORD_2D block, int block_size,
				 COORD_2D shift_global, int max_shift_local,
				 OPTICAL_FLOW* flow)
{
	double result;
	COORD_2D shift = {0, 0};

	int counter = 0;
	COORD_2D best_shift = shift;
	double min_result = diff_block (old_image, new_image, gui_image, block, shift, block_size);
	HISTOGRAM_STORAGE histogram[SQUARE(max_shift_local * 2 + 2)];
	histogram[counter].diff = min_result;
	histogram[counter].shift.x = 0;
	histogram[counter].shift.y = 0;
	counter++;

	if (min_result < flow->epsilon) return best_shift;

	for (int j = shift_global.y - max_shift_local; j <= shift_global.y + max_shift_local; j++) {
		for (int i = shift_global.x - max_shift_local; i <= shift_global.x + max_shift_local; i++) {
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
	best_shift = histogram[counter - 1].shift;

	if (median - min_result < flow->threshold) return (COORD_2D) {0, 0};

	int i = counter - 1;
	double best_distance = sqrt(SQUARE(histogram[i].shift.x) + SQUARE(histogram[i].shift.y));
	double distance;

	// some shift variants --- equal by "diff" value, so find shift variant with smallest distance to center
	while (i > 0 && histogram[i].diff - min_result < flow->histogram_epsilon) {
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
				 OPTICAL_FLOW* flow)
{
	int horizontal_blocks_num = get_block_numbers (new_image->width,  flow->block_size_in_pixel);
	int vertical_blocks_num   = get_block_numbers (new_image->height, flow->block_size_in_pixel);

	COORD_2D coord_shift;
	COORD_2D block;
	COORD_2DU pixel;
	long long int coord_raw;

	RGB_COLOR color_shift;
	int max_shift = sqrt(2.0 * (double)SQUARE (flow->max_shift_global + flow->max_shift_local));

	for (int j=0; j < vertical_blocks_num; j++) {
		block.y = j * flow->block_size_in_pixel;
		for (int i=0; i < horizontal_blocks_num; i++) {
			block.x = i * flow->block_size_in_pixel;
			int raw_flow_coord = coord_to_raw_flow(flow, (COORD_2DU) {.x=i, .y=j});
			if (raw_flow_coord >= 0) {
				coord_shift = find_block_correlation (old_image, new_image, gui_image,
								      block, flow->block_size_in_pixel,
								      flow->array[raw_flow_coord].shift, flow->max_shift_local, flow);

				for(pixel.y = block.y; pixel.y < (unsigned long int)(block.y + flow->block_size_in_pixel); pixel.y++) {
					for(pixel.x = block.x; pixel.x < (unsigned long int)(block.x + flow->block_size_in_pixel); pixel.x++) {
						coord_raw = coord_to_raw_chunk(gui_image, pixel);
						if (coord_raw >= 0) {
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
}



void *block_matching_optimized_images (void *vin)
{
	OPTICAL_FLOW* flow = vin;

	int counter = 0;

	int horizontal_blocks_num = flow->width;
	int vertical_blocks_num   = flow->height;

	COORD_2D coord_shift;
	COORD_2D block;



	/*
		printf("\n\n\nh=%d\n", horizontal_blocks_num);
	for (unsigned long int i = 0; i < flow->array_size; i++) {
		printf("%d ", flow->array[i].last_update);
		if (i%horizontal_blocks_num == 0) printf("\n");
	}
	*/





	// find in previous success blocks
	for (unsigned long int raw_flow_coord = 0; raw_flow_coord < flow->array_size; raw_flow_coord++) {
		if ((flow->array[raw_flow_coord].last_update == OPTICAL_FLOW_UPDATED_IN_PREVIOUS_ITERATION &&
		     !(flow->array[raw_flow_coord].shift.x == 0 && flow->array[raw_flow_coord].shift.y == 0)) ||
		     flow->array[raw_flow_coord].last_update > OPTICAL_FLOW_LONG_TIME_WITHOUT_UPDATE) { // update those that have not been updated for a long time
			    coord_shift = find_block_correlation (flow->old_image, flow->raw_image, flow->gui_image,
								  block, flow->block_size_in_pixel,
								  flow->array[raw_flow_coord].shift, flow->max_shift_local, flow);
			    flow->array[raw_flow_coord].shift = coord_shift;
			    flow->array[raw_flow_coord].last_update = OPTICAL_FLOW_JUST_UPDATED;
			    counter++;
		}
	}



	for (int j=0; j < vertical_blocks_num; j++) {
		block.y = j * flow->block_size_in_pixel;
		for (int i=0; i < horizontal_blocks_num; i++) {
			block.x = i * flow->block_size_in_pixel;
			int raw_flow_coord = coord_to_raw_flow(flow, (COORD_2DU) {.x=i, .y=j});
			if (raw_flow_coord >= 0) {
				coord_shift = flow->array[raw_flow_coord].shift;

				// color if most neighbour have shift
				if (flow->min_neighbours > 0 && coord_shift.x == 0 && coord_shift.y == 0) {
					COORD_2D neighbour_shift = {.x = 0, .y = 0};
					int neighbour_shift_counter = 0;
					for (int neighbour_x = -1; neighbour_x <= 1; neighbour_x++) {
						for (int neighbour_y = -1; neighbour_y <= 1; neighbour_y++) {
							int neighbour_coord = coord_to_raw_flow(flow, (COORD_2DU) {.x=i + neighbour_x, .y=j + neighbour_y});
							if (neighbour_coord >= 0 &&
							    flow->array[neighbour_coord].last_update != flow->painted_by_neighbor &&
							    (flow->array[neighbour_coord].shift.x != 0 ||
							     flow->array[neighbour_coord].shift.y != 0)) {
								neighbour_shift.x += flow->array[neighbour_coord].shift.x;
								neighbour_shift.y += flow->array[neighbour_coord].shift.y;
								neighbour_shift_counter++;
							}
						}
					}
					if (neighbour_shift_counter >= flow->min_neighbours) {
						//printf(" <%d> ", neighbour_shift_counter);
						coord_shift.x = neighbour_shift.x / neighbour_shift_counter;
						coord_shift.y = neighbour_shift.y / neighbour_shift_counter;

						flow->array[raw_flow_coord].shift = coord_shift;
						flow->array[raw_flow_coord].last_update = flow->painted_by_neighbor;
					}
				}
			}
		}
	}

	printf("%d:", counter);

	// process random block
	counter = 0;
	do {
		int i = rnd(0, horizontal_blocks_num - 1);
		int j = rnd(0, vertical_blocks_num - 1);
		int raw_flow_coord = coord_to_raw_flow(flow, (COORD_2DU) {.x=i, .y=j});
		if (raw_flow_coord >= 0 &&
		    flow->array[raw_flow_coord].last_update != OPTICAL_FLOW_JUST_UPDATED) {
			block.x = i * flow->block_size_in_pixel;
			block.y = j * flow->block_size_in_pixel;

			coord_shift = find_block_correlation (flow->old_image, flow->raw_image, flow->gui_image,
							      block, flow->block_size_in_pixel,
							      flow->array[raw_flow_coord].shift, flow->max_shift_local, flow); // generate a lot of trivial: shift(x,y) === 0
			flow->array[raw_flow_coord].shift = coord_shift;
			flow->array[raw_flow_coord].last_update = OPTICAL_FLOW_JUST_UPDATED;
			counter++;
		}

	} while (atomic_load(&(flow->semaphore_optical_flow)));
	printf("%d ", counter);

	/*
		printf("\n\n\nh=%d\n", horizontal_blocks_num);
	for (unsigned long int i = 0; i < flow->array_size; i++) {
		printf("%d ", flow->array[i].last_update);
		if (i%horizontal_blocks_num == 0) printf("\n");
	}
	*/


/*
	// neighbour
	for (unsigned long int j = 1; j < flow->height -1; j++) {
		for (unsigned long int i = 1; i < flow->width-1; i++) {
			COORD_2DU coord = {.x=i, .y=j};
			long long int index = coord_to_raw_flow(flow, coord);


			COORD_2D coord_shift = flow->array[raw_flow_coord].shift;
			// color if most neighbour have shift
			if (MIN_NEIGHBOURS > 0 && coord_shift.x == 0 && coord_shift.y == 0) {
				COORD_2D neighbour_shift = {.x = 0, .y = 0};
				int neighbour_shift_counter = 0;
				for (int neighbour_x = -1; neighbour_x <= 1; neighbour_x++) {
					for (int neighbour_y = -1; neighbour_y <= 1; neighbour_y++) {
						int neighbour_coord = coord_to_raw_flow(flow, (COORD_2DU) {.x=i + neighbour_x, .y=j + neighbour_y});
						if (neighbour_coord >= 0 &&
						    (flow->array[neighbour_coord].shift.x != 0 ||
						     flow->array[neighbour_coord].shift.y != 0)) {
							neighbour_shift.x += flow->array[neighbour_coord].shift.x;
							neighbour_shift.y += flow->array[neighbour_coord].shift.y;
							neighbour_shift_counter++;
						}
					}
				}
				if (neighbour_shift_counter >= MIN_NEIGHBOURS) {
					coord_shift.x = neighbour_shift.x / neighbour_shift_counter;
					coord_shift.y = neighbour_shift.y / neighbour_shift_counter;
				}
			}
		}
	}
*/

	for (unsigned long int i = 0; i < flow->array_size; i++) {
		flow->array[i].last_update += 1;
	}



	colorize(flow->raw_image, flow->gui_image, flow);

	return 0;
}



void colorize (struct imgRawImage* new_image, struct imgRawImage* gui_image, OPTICAL_FLOW* flow)
{
	extern int hide_static_block;

	int horizontal_blocks_num = flow->width;
	int vertical_blocks_num   = flow->height;

	COORD_2D coord_shift;
	COORD_2D block;
	COORD_2DU pixel;
	long long int coord_raw;

	RGB_COLOR color_shift;

	int max_shift = sqrt(2.0 * (double)SQUARE (flow->max_shift_global + flow->max_shift_local));

	coord_shift = (COORD_2D) {.x=0, .y=0};
	for (int j=0; j < vertical_blocks_num; j++) {
		block.y = j * flow->block_size_in_pixel;
		for (int i=0; i < horizontal_blocks_num; i++) {
			block.x = i * flow->block_size_in_pixel;
			int raw_flow_coord = coord_to_raw_flow(flow, (COORD_2DU) {.x=i, .y=j});
			if (raw_flow_coord >= 0) {
				coord_shift = flow->array[raw_flow_coord].shift;

				/*
				// color if most neighbour have shift
				if (MIN_NEIGHBOURS > 0 && coord_shift.x == 0 && coord_shift.y == 0) {
					COORD_2D neighbour_shift = {.x = 0, .y = 0};
					int neighbour_shift_counter = 0;
					for (int neighbour_x = -1; neighbour_x <= 1; neighbour_x++) {
						for (int neighbour_y = -1; neighbour_y <= 1; neighbour_y++) {
							int neighbour_coord = coord_to_raw_flow(flow, (COORD_2DU) {.x=i + neighbour_x, .y=j + neighbour_y});
							if (neighbour_coord >= 0 &&
							    flow->array[neighbour_coord].last_update != PAINTED_BY_NEIGHBOR &&
							    (flow->array[neighbour_coord].shift.x != 0 ||
							     flow->array[neighbour_coord].shift.y != 0)) {
								neighbour_shift.x += flow->array[neighbour_coord].shift.x;
								neighbour_shift.y += flow->array[neighbour_coord].shift.y;
								neighbour_shift_counter++;
							}
						}
					}
					if (neighbour_shift_counter >= MIN_NEIGHBOURS) {
						//printf(" <%d> ", neighbour_shift_counter);
						coord_shift.x = neighbour_shift.x / neighbour_shift_counter;
						coord_shift.y = neighbour_shift.y / neighbour_shift_counter;

						flow->array[raw_flow_coord].shift = coord_shift;
						flow->array[raw_flow_coord].last_update = PAINTED_BY_NEIGHBOR;
					}
				}
				*/

				for(pixel.y = block.y; pixel.y < (unsigned long int)(block.y + flow->block_size_in_pixel); pixel.y++) {
					for(pixel.x = block.x; pixel.x < (unsigned long int)(block.x + flow->block_size_in_pixel); pixel.x++) {
						coord_raw = coord_to_raw_chunk(gui_image, pixel);
						if (coord_raw >= 0) {
							RGB_COLOR source_color = {
								.r = new_image->lpData[coord_raw + R],
								.g = new_image->lpData[coord_raw + G],
								.b = new_image->lpData[coord_raw + B]};
							if (coord_shift.x == 0 && coord_shift.y == 0) {
								unsigned char mono = monochrome(source_color);
								if (hide_static_block == true) {
									mono = (mono>>2) + 255 - (255>>2);
								}
								gui_image->lpData[coord_raw + R] = mono;
								gui_image->lpData[coord_raw + G] = mono;
								gui_image->lpData[coord_raw + B] = mono;
							} else {
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
	}

	/*
	// not faster than code above
	int raw_flow_coord = 0;
	int raw_counter = 0;
	for (long unsigned int coord_raw = 0; coord_raw < gui_image->dwBufferBytes; coord_raw += gui_image->numComponents) {
		RGB_COLOR source_color = {
			.r = new_image->lpData[coord_raw + R],
			.g = new_image->lpData[coord_raw + G],
			.b = new_image->lpData[coord_raw + B]};

		struct coord_2Du image_coord = raw_chunk_to_coord(gui_image, coord_raw);
		image_coord.x /= flow->block_size_in_pixel;
		image_coord.y /= flow->block_size_in_pixel;
		long long int raw_flow_coord = coord_to_raw_flow(flow, image_coord);

		coord_shift = flow->array[raw_flow_coord].shift;
		if (coord_shift.x == 0 && coord_shift.y == 0) {
			unsigned char mono = monochrome(source_color);
			if (hide_static_block == true) {
				mono = (mono>>2) + 255 - (255>>2);
			}
			gui_image->lpData[coord_raw + R] = mono;
			gui_image->lpData[coord_raw + G] = mono;
			gui_image->lpData[coord_raw + B] = mono;
		} else {
			color_shift = shift_to_color (source_color, coord_shift, max_shift);
			gui_image->lpData[coord_raw + R] = color_shift.r;
			gui_image->lpData[coord_raw + G] = color_shift.g;
			gui_image->lpData[coord_raw + B] = color_shift.b;
		}
	}
	*/
}



unsigned char monochrome (RGB_COLOR source_color)
{
	return (0.2125 * (double)source_color.r) + (0.7154 * (double)source_color.g) + (0.0721 * (double)source_color.b);
}


RGB_COLOR shift_to_color (RGB_COLOR source_color, COORD_2D shift, int max_shift)
{
	double mono = monochrome(source_color);

	// HSL (for hue, saturation, lightness) and HSV (for hue, saturation, value; also known as HSB, for hue, saturation, brightness)
	// convert HSL to RGB
	// Given a color with hue H [0, 360], saturation S [0, 1], and lightness L [0, 1]
	double hue = convert_radian_to_degree(angle_modulo(atan2(shift.y, shift.x)));
	double saturation = sqrt((double)SQUARE(shift.x) + (double)SQUARE(shift.y)) / (double) max_shift;
	double lightness = mono / 255.0;

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
