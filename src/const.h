/** \file
   const.h --- header with const and enum

   Copyright (C) 2021 Roman V. Prikhodchenko

   Author: Roman V. Prikhodchenko <chujoii@gmail.com>
*/

// include guard
#ifndef CONST_H
#define CONST_H


// optical flow
#define OPTICAL_FLOW_EPSILON 1.0e-3
#define OPTICAL_FLOW_HISTOGRAM_EPSILON 1.0e-1
#define OPTICAL_FLOW_THRESHOLD 2

#define OPTICAL_FLOW_BLOCK_SIZE 8
#define OPTICAL_FLOW_MAX_SHIFT_GLOBAL 8    // shift_global === previoush shift
#define OPTICAL_FLOW_MAX_SHIFT_LOCAL 4     // shift_local === distance from shift_global for search similar
#define OPTICAL_FLOW_MIN_NEIGHBOURS 3
#define OPTICAL_FLOW_FPS 15
#define OPTICAL_FLOW_LONG_TIME_WITHOUT_UPDATE 30
#define OPTICAL_FLOW_PAINTED_BY_NEIGHBOR 40





#define OPTICAL_FLOW_JUST_UPDATED 0
#define OPTICAL_FLOW_UPDATED_IN_PREVIOUS_ITERATION 1


#define NANOSECONDS_IN_SECOND 1000000000L
#define SECONDS_IN_DEGREE 60.0

enum verbose_level {
	VERBOSE_NO = 0x00,
	VERBOSE_IMAGE = 0x01,
	VERBOSE_VIDEO = 0x02,
	VERBOSE_STEP_BY_STEP = 0x04
};
enum verbose_type { VERBT_GRADER, VERBT_FILTER, VERBT_SECONDDERIVATIVE, VERBT_LAST};
enum boolean {false, true};

enum wing_size {
	NO_FILTER_WING_EFFECTIVE_SIZE_MULTIPLIER = 2,
	MEDIAN_WING_EFFECTIVE_SIZE_MULTIPLIER = 2,
	ADDITIONAL_MHAT_WAVELET_WING_EFFECTIVE_SIZE_MULTIPLIER = 1
};

enum filter {NO_FILTER, MEDIAN, WAVELET};

enum mouse_state_enum {NO_POINT_SET, ZERO_POINT_SET, FIRST_POINT_SET, SECOND_POINT_SET};

#endif /* CONST_H */
