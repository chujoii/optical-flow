/** \file
unit-testing.c --- Unit-testing block-matching f-or optical-flow algo-rithm

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

#include <stdio.h>

#include "../../../ABMI.00232/src/const.h"
#include "block-matching.h"


int verbose = VERBOSE_NO;
int escape_status = false;
int status_coord_of_robot = false;
int mouse_state = NO_POINT_SET;
int glob_filter_type = MEDIAN;

struct imgRawImage* raw_image;
struct imgRawImage* gui_image;
struct imgRawImage* old_image;

int main (int argc, char **argv)
{
	printf("hello world\n");
	return 0;
}
