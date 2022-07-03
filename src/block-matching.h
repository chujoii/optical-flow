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
double diff_block (struct imgRawImage* old_image, struct imgRawImage* new_image, COORD_2D block, COORD_2D shift, int block_size, int debug);
COORD_2D find_block_correlation (struct imgRawImage* old_image, struct imgRawImage* new_image, COORD_2D block, int max_shift, int block_size);
void compare_full_images (struct imgRawImage* old_image, struct imgRawImage* new_image, int max_shift, int block_size);

#endif /* BLOCK_MATCHING_H */
