/** \file
   util.h --- header for util.c

   Copyright (C) 2021 Roman V. Prikhodchenko

   Author: Roman V. Prikhodchenko <chujoii@gmail.com>
*/

// include guard
#ifndef UTIL_H
#define UTIL_H

#define SQUARE(x) ((x) * (x))

#define CUBE(x) ((x) * (x) * (x))

#define SIGN(a) ( ( (a) < 0 )  ?  -1   : ( (a) > 0 ) )

#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define MAX(A, B) ((A) > (B) ? (A) : (B))

int int_constrain(int val, int min_val, int max_val);
float float_constrain(float val, float min_val, float max_val);
float convert_radian_to_degree(float a);
float convert_degree_to_radian(float a);
struct coord_2Df move_point_along_by_vector (float x, float y, float angle_rad, float distance);
struct coord_2Df move_point_transverse_to_vector (float x, float y, float angle_rad, float distance);
struct coord_2Df isometry_forward_shift_and_rotate (float x, float shift_x, float y, float shift_y, float sin_alpha, float cos_alpha);
struct coord_2Df isometry_backward_shift_and_rotate (float x, float shift_x, float y, float shift_y, float sin_alpha, float cos_alpha);
float get_angle_by_2_point (struct coord_2Df start, struct coord_2Df end);
float distance_between_2_point (struct coord_2Df start, struct coord_2Df end);
float angle_modulo (float angle);
float opening_angle (float a, float b);
float first_derivative (float * array, int i, int len_array);
float second_derivative (float * array, int i, int len_array);
int search_index_of_nearest_point (int len_array, float * array_x, float * array_y, float search_val_x, float search_val_y, int start_point_index, float square_error);
int rnd(int min, int max);

#endif /* UTIL_H */
