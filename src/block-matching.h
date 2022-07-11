/** \file
   block-matching.h --- header for block-matching.c

   Copyright (C) 2022 Roman V. Prikhodchenko

   Author: Roman V. Prikhodchenko <chujoii@gmail.com>
*/

// include guard
#ifndef BLOCK_MATCHING_H
#define BLOCK_MATCHING_H

#include "image-type.h"

void print_image (struct imgRawImage* image);
int get_block_numbers (int image_size, int block_size);
double diff_block (struct imgRawImage* old_image, struct imgRawImage* new_image, struct imgRawImage* gui_image,
		   COORD_2D block, COORD_2D shift, int block_size);
COORD_2D find_block_correlation (struct imgRawImage* old_image, struct imgRawImage* new_image, struct imgRawImage* gui_image, COORD_2D block, int max_shift, int block_size);
void block_matching_full_images (struct imgRawImage* old_image, struct imgRawImage* new_image, struct imgRawImage* gui_image,
				 int max_shift, int block_size);
RGB_COLOR shift_to_color (RGB_COLOR source_color, COORD_2D shift, int max_shift);

#endif /* BLOCK_MATCHING_H */
