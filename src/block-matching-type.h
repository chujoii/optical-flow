/** \file
   block-matching-type.h --- header for block-matching.c

   Copyright (C) 2021 Roman V. Prikhodchenko

   Author: Roman V. Prikhodchenko <chujoii@gmail.com>
*/

// include guard
#ifndef BLOCK_MATCHING_TYPE_H
#define BLOCK_MATCHING_TYPE_H



typedef struct blk {
	COORD_2D shift;
	double diff;
	int last_update;
} BLK;

typedef struct optical_flow {
	int block_size_in_pixel;
	int max_shift_global;
	int max_shift_local;
	long int nspf; // inverted frames per seconds == nanoseconds per frame

	unsigned long int width;
	unsigned long int height;
	unsigned long int array_size;
	BLK* array;
} OPTICAL_FLOW;

typedef struct histogram_storage {
	COORD_2D shift;
	double diff;
} HISTOGRAM_STORAGE;


#endif /* BLOCK_MATCHING_TYPE_H */
