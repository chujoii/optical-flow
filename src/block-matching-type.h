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
	int last_update; // 0:  recently updated;        >0 (1, 2, 3, ...): updated in previous iteration;
} BLK;

typedef struct optical_flow {
	int block_size_in_pixel;
	int max_shift_global; // shift_global === previoush shift
	int max_shift_local; // shift_local === distance from shift_global for search similar
	long int nspf; // inverted frames per seconds == nanoseconds per frame
	double epsilon;
	double histogram_epsilon;
	double threshold;
	int min_neighbours;
	int long_time_without_update;
	int painted_by_neighbor;

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
