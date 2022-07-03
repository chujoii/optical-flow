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

#include "const.h"
#include "image.h"
#include "block-matching.h"




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
double diff_block (struct imgRawImage* old_image, struct imgRawImage* new_image, COORD_2D block, COORD_2D shift, int block_size, int debug)
{
	//struct imgRawImage* raw_image; // fixme: global variable
        extern struct imgRawImage* gui_image; // fixme: global variable
        //struct imgRawImage* old_image; // fixme: global variable

	unsigned long int sum = 0;
	unsigned long int counter = 0;

	// fixme: add simultaneous rotation and translation
	COORD_2D coord_2d_old;
	COORD_2D coord_2d_new;
	long long int coord_raw_old;
	long long int coord_raw_new;
	int nx, ny; // index for new image

	for (ny = 0; ny < block_size; ny++) {
		coord_2d_old.y = block.y + ny + shift.y;
		coord_2d_new.y = block.y + ny;
		for (nx = 0; nx < block_size; nx++) {
			coord_2d_old.x = block.x + nx + shift.x;
			coord_2d_new.x = block.x + nx;

			if (coord_2d_old.x >= 0 && coord_2d_old.y >= 0 &&
			    coord_2d_new.x >= 0 && coord_2d_new.y >= 0) {
				COORD_2DU coord_2du_old = {coord_2d_old.x, coord_2d_old.y};
				COORD_2DU coord_2du_new = {coord_2d_new.x, coord_2d_new.y};
				coord_raw_old = coord_to_raw_chunk(old_image, coord_2du_old);
				coord_raw_new = coord_to_raw_chunk(new_image, coord_2du_new);

				if (coord_raw_old >= 0 && coord_raw_new >= 0) {
					for (unsigned int color = 0; color < new_image->numComponents; color++) {
						if (debug == true) {
							gui_image->lpData[coord_raw_old + color] += 20;
							gui_image->lpData[coord_raw_new + color] += 1;
						}
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


/**
   Find block correlation with all possible shifts
*/
COORD_2D find_block_correlation (struct imgRawImage* old_image, struct imgRawImage* new_image, COORD_2D block, int max_shift, int block_size)
{
	double result;
	COORD_2D shift = {0, 0};

	COORD_2D best_shift = shift;
	double min_result = diff_block (old_image, new_image, block, shift, block_size, false);
	if (min_result < EPSILON) return best_shift;

	for (int j = -max_shift; j <= max_shift; j++) {
		for (int i = -max_shift; i <= max_shift; i++) {
			shift.x = i; shift.y = j;
			result = diff_block (old_image, new_image, block, shift, block_size, false);
			if (result < min_result) {
				min_result = result;
				best_shift = shift;
				if (min_result < EPSILON) return best_shift;
			}
		}
	}
	return best_shift;
}



void compare_full_images (struct imgRawImage* old_image, struct imgRawImage* new_image, int max_shift, int block_size)
{
        extern struct imgRawImage* gui_image; // fixme: global variable

	int horizontal_blocks_num = new_image->width / block_size + ((new_image->width % block_size > 0) ? 1 : 0);
	int vertical_blocks_num = new_image->height / block_size + ((new_image->height % block_size > 0) ? 1 : 0);

	COORD_2D coord_shift;
	COORD_2D block;
	COORD_2DU pixel;
	long long int coord_raw;

	for (int j=0; j < vertical_blocks_num; j++) {
		block.y = j * block_size;
		for (int i=0; i < horizontal_blocks_num; i++) {
			block.x = i * block_size;
			coord_shift = find_block_correlation (old_image, new_image, block, max_shift, block_size);

			for(pixel.y = block.y; pixel.y < (unsigned long int)(block.y + block_size); pixel.y++) {
				for(pixel.x = block.x; pixel.x < (unsigned long int)(block.x + block_size); pixel.x++) {
					coord_raw = coord_to_raw_chunk(gui_image, pixel);
					if (coord_raw > 0) {
						for (unsigned int color = 0; color < new_image->numComponents; color++) {
							gui_image->lpData[coord_raw + color] = abs((int)coord_shift.x) + abs((int)coord_shift.y);
						}
					}
				}
			}
			
		}
	}
}
