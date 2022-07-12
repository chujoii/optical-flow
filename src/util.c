/** \file
util.c --- utils module

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



Keywords: util constrain simple geometry functions

Usage:

History:

Code:
*/

#include <math.h>

#include "util.h"
#include "gui.h"



//#define DEBUG
// add to project previous string for debug=ON
// change printf(x) -> fprintf(stderr, x)
#ifdef DEBUG
 #define DEBUG_PRINT(x)  printf x
#else
 #define DEBUG_PRINT(x)
#endif



int int_constrain(int val, int min_val, int max_val)
{
	return MIN(max_val, MAX(min_val, val));
}

float float_constrain(float val, float min_val, float max_val)
{
	return fmin(max_val, fmax(min_val, val));
}



float convert_radian_to_degree(float a)
{
	return a * 180.0 / M_PI;
}

float convert_degree_to_radian(float a)
{
	return a * M_PI / 180.0;
}



/**
   Move point along by vector
*/
struct coord_2Df move_point_along_by_vector (float x, float y, float angle_rad, float distance)
{
	struct coord_2Df c;
	c.x = x + distance * cos(angle_rad);
	c.y = y + distance * sin(angle_rad);
	return c;
}



/**
   Move point transverse to vector
*/
struct coord_2Df move_point_transverse_to_vector (float x, float y, float angle_rad, float distance)
{
	struct coord_2Df c;
	c.x = x + distance * cos(angle_rad + M_PI/2);
	c.y = y + distance * sin(angle_rad + M_PI/2);
	return c;
}


/** Isometry forward shift and rotate
   X = x'*cos(a) - y'*sin(a) + x0
   Y = x'*sin(a) + y'*cos(a) + y0
*/
struct coord_2Df isometry_forward_shift_and_rotate (float x, float shift_x, float y, float shift_y, float sin_alpha, float cos_alpha)
{
	struct coord_2Df new_coord;
	new_coord.x = x*cos_alpha - y*sin_alpha + shift_x;
	new_coord.y = x*sin_alpha + y*cos_alpha + shift_y;
	return new_coord;
}



/** Isometry backward shift and rotate
   x' =  (X-x0)*cos(a) + (Y-y0)*sin(a)
   y' = -(X-x0)*sin(a) + (Y-y0)*cos(a)
*/
struct coord_2Df isometry_backward_shift_and_rotate (float x, float shift_x, float y, float shift_y, float sin_alpha, float cos_alpha)
{
	struct coord_2Df old_coord;
	old_coord.x =  (x - shift_x)*cos_alpha + (y - shift_y)*sin_alpha;
	old_coord.y = -(x - shift_x)*sin_alpha + (y - shift_y)*cos_alpha;
	return old_coord;
}


/** Return angle, calculated by two points. (and horizontal)

    The point of atan2() is that the signs of both inputs are known to
    it, so it can compute the correct quadrant for the angle.
*/
float get_angle_by_2_point (struct coord_2Df start, struct coord_2Df end)
{
	return atan2((end.y - start.y), (end.x - start.x));
	//return atan((end.y - start.y) / (end.x - start.x));
}



float distance_between_2_point (struct coord_2Df start, struct coord_2Df end)
{
	return sqrt(SQUARE(end.x - start.x) + SQUARE(end.y - start.y));
}



float angle_modulo (float angle)
{
	float new_angle = fmod(angle, 2.0*M_PI); // %
	if (new_angle < 0) {new_angle += (2.0*M_PI);}
	return new_angle;
}

float opening_angle (float a, float b)
{
	a = angle_modulo(a);
	b = angle_modulo(b);
	float minimum = MIN(a, b);
	float maximum = MAX(a, b);
	return MIN(maximum - minimum, (2.0*M_PI - maximum) + minimum);
}



float first_derivative (float * array, int i, int len_array)
{
	if (i < 0)
		return first_derivative (array, 0, len_array);

	if (i > len_array - 2)
		return first_derivative (array, len_array - 2, len_array);

	return array[i+1] - array[i];
}



float second_derivative (float * array, int i, int len_array)
{
	if (i < 0)
		return second_derivative (array, 0, len_array);

	if (i > len_array - 3)
		return second_derivative (array, len_array - 3, len_array);

	return array[i] - 2*array[i+1] + array[i+2];
}



int search_index_of_nearest_point (int len_array, float * array_x, float * array_y, float search_val_x, float search_val_y, int start_point_index, float square_error)
{
	start_point_index = int_constrain(start_point_index, 0, len_array - 1);
	int left  = start_point_index;
	int right = start_point_index;
	float min_square_distance =
		SQUARE(array_x[start_point_index]  - search_val_x) +
		SQUARE(array_y[start_point_index]  - search_val_y);
	int min_index = start_point_index;

	while (!(left <= 0 && right >= len_array - 1) &&
	       min_square_distance < square_error) {
		float square_distance_left =
			SQUARE(array_x[left]  - search_val_x) +
			SQUARE(array_y[left]  - search_val_y);
		float square_distance_right =
			SQUARE(array_x[right] - search_val_x) +
			SQUARE(array_y[right] - search_val_y);
		if (min_square_distance > square_distance_left) {
			min_square_distance = square_distance_left;
			min_index = left;
		}
		if (min_square_distance > square_distance_right) {
			min_square_distance = square_distance_right;
			min_index = right;
		}

		if (left  > 0) left--;
		if (right < len_array - 1) right++;
	}

	return min_index;
}



/**
   see https://stackoverflow.com/questions/2509679/how-to-generate-a-random-integer-number-from-within-a-range
*/
int rnd(int min, int max)
{
	// for float 	r = (double)rand() / (double)(RAND_MAX);

	int r;
	int range = 1 + max - min;
	int buckets = RAND_MAX / range;
	int limit = buckets * range;

	do {
		r = rand();
	} while (r >= limit);

	return min + (r / buckets);
}
