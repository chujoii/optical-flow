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

#include "../../../ABMI.00232/src/speckled-band.h"
#include "../../../ABMI.00232/src/image.h"
#include "../../../ABMI.00232/src/capture.h"
#include "../../../ABMI.00232/src/lookout.h"
#include "../../../ABMI.00232/src/gui.h"
#include "../../../ABMI.00232/src/util.h"

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
unsigned long int diff_block (coord_2Du old, coord_2Du new, int block_size)
{
	extern struct imgRawImage* raw_image; // fixme: global variable
        extern struct imgRawImage* gui_image; // fixme: global variable
        extern struct imgRawImage* old_image; // fixme: global variable

	unsigned long int sum = 0;
	unsigned long int counter = 0;

	coord_2Du coord_2d_old;
	coord_2Du coord_2d_new;
	unsigned long int coord_raw_old;
	unsigned long int coord_raw_new;
	int ox, oy; // index for old image
	int nx, ny; // index for new image


	int shift_x = new.x - old.x;
	int shift_y = new.y - old.y;

	for (nx = 0; nx < block_size; nx++) { // new image
		ox = nx - shift_x;
		coord_2d_old.x = old.x + ox;
		coord_2d_new.x = new.x + nx;
		for (ny = 0; ny < block_size; ny++) { // new image
			oy = ny - shift_y;
			coord_2d_old.y = old.y + oy;
			coord_2d_new.y = new.y + ny;

			coord_raw_old = coord_to_raw_chunk(old_image->width, coord_2d_old);
			coord_raw_new = coord_to_raw_chunk(raw_image->width, coord_2d_new);

			if (coord_raw_old < old_image->dwBufferBytes && coord_raw_new < raw_image->dwBufferBytes) { // coord_raw_xxx always > 0: because unsigned
				for (unsigned int color = 0; color < raw_image->numComponents; color++) {
					sum += abs(old_image[coord_raw_old + color] - raw_image[coord_raw_new + color]);
					counter++;
				}
			}
		}
	}

	if (counter == 0) return 0;
	return sum/counter;
}
