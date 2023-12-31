/** \file
   block-matching.h --- header for block-matching.c

   Copyright (C) 2022 Roman V. Prikhodchenko

   Author: Roman V. Prikhodchenko <chujoii@gmail.com>
*/

// include guard
#ifndef BLOCK_MATCHING_H
#define BLOCK_MATCHING_H

#include "image-type.h"
#include "block-matching-type.h"

long long int coord_to_raw_flow(OPTICAL_FLOW * flow, COORD_2DU coord);
COORD_2DU raw_flow_to_coord(OPTICAL_FLOW* flow, unsigned long int r);
void print_image (struct imgRawImage* image);
int init_block_matching (int image_width, int image_height, int block_size, int max_shift_global, int max_shift_local, double epsilon, double histogram_epsilon, double threshold, int min_neighbours, int long_time_without_update, int painted_by_neighbor, OPTICAL_FLOW* flow);
void free_block_matching (OPTICAL_FLOW* flow);
int get_block_numbers (int image_size, int block_size);
double diff_block (struct imgRawImage* old_image, struct imgRawImage* new_image, struct imgRawImage* gui_image,
		   COORD_2D block, COORD_2D shift, int block_size);
COORD_2D find_block_correlation (struct imgRawImage* old_image, struct imgRawImage* new_image, struct imgRawImage* gui_image,
				 COORD_2D block, int block_size, 
				 COORD_2D shift_global, int max_shift_local, OPTICAL_FLOW* flow);
void block_matching_full_images (struct imgRawImage* old_image, struct imgRawImage* new_image, struct imgRawImage* gui_image,
				 OPTICAL_FLOW* flow);
void *block_matching_optimized_images (void *vin);
void colorize (struct imgRawImage* new_image, struct imgRawImage* gui_image, OPTICAL_FLOW* flow);
unsigned char monochrome (RGB_COLOR source_color);
RGB_COLOR shift_to_color (RGB_COLOR source_color, COORD_2D shift, int max_shift);

#endif /* BLOCK_MATCHING_H */
