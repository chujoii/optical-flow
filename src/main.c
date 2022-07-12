/** \file
main.c --- display block-matching result for optical-flow algorithm

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
#include <getopt.h>          /* getopt_long() */
#include <time.h>

#include "const.h"
#include "capture.h"
#include "gui.h"
#include "block-matching.h"
#include "main.h"



static const char short_options[] = "d:hoyn:v:";

static const struct option
long_options[] = {
        // english alphabet:  abcdefghijklmnopqrstuvwxyz.
        // used                  x   x     xx      x  x .
        { "device",             required_argument, NULL, 'd' },
        { "help",               no_argument,       NULL, 'h' },
        { "output",             no_argument,       NULL, 'o' },
        { "yuyv",               no_argument,       NULL, 'y' },
        { "numframes",          required_argument, NULL, 'n' },
        { "verbose",            required_argument, NULL, 'v' },
        { 0, 0, 0, 0 }
};



void usage(FILE *fp, char **argv, char *dev_name, int frame_count)
{
        fprintf(fp,
                "Usage: %s [options]\n\n"
                "Version 1.3\n"
                "Options:\n"
                "-d | --device name             Video device name [%s]\n"
                "-h | --help                    Print this message\n"
                "-o | --output                  Outputs stream to stdout\n"
                "-y | --yuyv                    Force format to 640x480 YUYV\n"
                "-n | --numframes               Number of frames to grab [current value = %i].\n"
                "\t\t\tIf \"n\" not specified, or \"n\" less than 1 (zero or negative),\n"
                "\t\t\t then loop will continue infinity\n"
                "-v | --verbose                 Verbose: without only text print;\n"
                "\t\t\t'-v 1' 0x01=b00000001 add jpeg;\n"
                "\t\t\t'-v 2' 0x02=b00000010 add video;\n"
                "\t\t\t'-v 4' 0x04=b00000100 add step by step      (you can ON this function by press 'space')\n"
                "\t\t\tso -v 6: mean verbose level 0x06=b00000110 that equal to both video and step_by_step\n"
                "\n"
                "\t\t\t1 variant\n"
                "\t\t\tstatic coordinates:\n"
                "\n"
                "\t\t\t2 variant\n"
                "\t\t\tserver specified\n"
                "\n",
                argv[0], dev_name, frame_count);
}



int main (int argc, char **argv)
{
	char *dev_name = "/dev/video0";
	int out_buf = 0;
	int force_format = 0;
	int max_frame_count = -1;

	unsigned int WINDOW_WIDTH = 640;
        unsigned int WINDOW_HEIGHT = 360;

        for (;;) {
                int idx;
                int c;

                c = getopt_long(argc, argv,
                                short_options, long_options, &idx);

                if (-1 == c)
                        break;

                switch (c) {
                case 0: /* getopt_long() flag */
                        break;

                case 'd':
                        dev_name = optarg;
                        break;

                case 'h':
                        usage(stdout, argv, dev_name, max_frame_count);
                        exit(EXIT_SUCCESS);

                case 'o':
                        out_buf++;
                        break;

                case 'y':
                        force_format++;
                        break;

                case 'n':
                        errno = 0;
                        max_frame_count = strtol(optarg, NULL, 0);
                        if (errno)
                                printf("%s", optarg);
                        break;

                case 'v':
                        verbose = strtol(optarg, NULL, 0);
                        if (errno)
                                printf("%s", optarg);
                        break;

                default:
                        usage(stderr, argv, dev_name, max_frame_count);
                        exit(EXIT_FAILURE);
                }
        }


	// init gui: OpenGL(glfw)
        if (verbose & VERBOSE_VIDEO) {
                int ret = init_gui(WINDOW_WIDTH, WINDOW_HEIGHT);
                printf("init_gui = %d", ret);
                video_texture = init_shader_video();
        }


	srandom((unsigned int)time(NULL));

	mainloop(dev_name, max_frame_count, video_texture);
	return 0;
}
