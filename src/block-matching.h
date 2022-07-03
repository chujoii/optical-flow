/** \file
   block-matching.h --- header for block-matching.c

   Copyright (C) 2022 Roman V. Prikhodchenko

   Author: Roman V. Prikhodchenko <chujoii@gmail.com>
*/

// include guard
#ifndef BLOCK_MATCHING_H
#define BLOCK_MATCHING_H

#include "image-type.h"

unsigned long int diff_block (struct imgRawImage* old_image, COORD_2DU old, struct imgRawImage* new_image, COORD_2DU new, int block_size);

#endif /* BLOCK_MATCHING_H */
