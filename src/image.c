/** \file
read-image.c --- libjpeg reads an JPEG file into an bitmap buffer

Copyright (C) 2022 Roman V. Prikhodchenko

Author: Roman V. Prikhodchenko <chujoii@gmail.com>,
	Thomas Spielauer, Wien (webcomplains389t48957@tspi.at) (https://www.tspi.at/2020/03/20/libjpegexample.html),
	Kenneth Finnegan <PhirePhly> (https://gist.github.com/PhirePhly/3080633)
	ReachConnection and nschmidt (https://stackoverflow.com/questions/1106741/display-yuv-in-opengl)
	yuripourre (https://github.com/yuripourre/V4GL/blob/master/Camera.cpp)

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



Keywords: libjpeg read jpeg bitmap buffer

Usage:

History:
    based on:
    libjpeg:
    https://www.tspi.at/2020/03/20/libjpegexample.html
    https://gist.github.com/PhirePhly/3080633

    yuv:
    https://stackoverflow.com/questions/1106741/display-yuv-in-opengl
    https://github.com/yuripourre/V4GL/blob/master/Camera.cpp

Code:
*/

#include <stdlib.h>
#include <stdio.h>
#include <jpeglib.h>
#include <jerror.h>

#include "image-type.h"
#include "image.h"
#include "const.h"
#include "gui.h"
#include "block-matching.h"

struct imgRawImage* loadJpegImageFile(char* lpFilename) {
	struct jpeg_decompress_struct info;
	struct jpeg_error_mgr err;

	struct imgRawImage* lpNewImage;

	unsigned long int imgWidth, imgHeight;
	int numComponents;

	unsigned long int dwBufferBytes;
	unsigned char* lpData;

	unsigned char* lpRowBuffer[1];

	FILE* fHandle;

	fHandle = fopen(lpFilename, "rb");
	if(fHandle == NULL) {
		#ifdef DEBUG
			fprintf(stderr, "%s:%d: Failed to read file %s\n", __FILE__, __LINE__, lpFilename);
		#endif
		return NULL; /* ToDo */
	}

	info.err = jpeg_std_error(&err);
	jpeg_create_decompress(&info);

	jpeg_stdio_src(&info, fHandle);
	jpeg_read_header(&info, TRUE);

	jpeg_start_decompress(&info);
	imgWidth = info.output_width;
	imgHeight = info.output_height;
	numComponents = info.num_components;

	#ifdef DEBUG
		fprintf(
			stderr,
			"%s:%d: Reading JPEG with dimensions %lu x %lu and %d components\n",
			__FILE__, __LINE__,
			imgWidth, imgHeight, numComponents
		);
	#endif

	dwBufferBytes = imgWidth * imgHeight * 3; /* We only read RGB, not A */
	lpData = (unsigned char*)malloc(sizeof(unsigned char)*dwBufferBytes);

	lpNewImage = (struct imgRawImage*)malloc(sizeof(struct imgRawImage));
	lpNewImage->numComponents = numComponents;
	lpNewImage->width = imgWidth;
	lpNewImage->height = imgHeight;
	lpNewImage->lpData = lpData;

	/* Read scanline by scanline */
	while(info.output_scanline < info.output_height) {
		lpRowBuffer[0] = (unsigned char *)(&lpData[3*info.output_width*info.output_scanline]);
		jpeg_read_scanlines(&info, lpRowBuffer, 1);
	}

	jpeg_finish_decompress(&info);
	jpeg_destroy_decompress(&info);
	fclose(fHandle);

	return lpNewImage;
}


/* modified: see loadJpegImageFile */
struct imgRawImage* loadJpegImage(const void *jpg_buffer, int jpg_size) {
	int rc;

	struct jpeg_decompress_struct info;
	struct jpeg_error_mgr err;

	struct imgRawImage* lpNewImage;

	unsigned long int imgWidth, imgHeight;
	int numComponents;

	unsigned long int dwBufferBytes;
	unsigned char* lpData;

	unsigned char* lpRowBuffer[1];


	info.err = jpeg_std_error(&err);
	jpeg_create_decompress(&info);

	jpeg_mem_src(&info, jpg_buffer, jpg_size);
	rc = jpeg_read_header(&info, TRUE);
	if (rc != 1) {
		printf("File does not seem to be a normal JPEG");
		exit(EXIT_FAILURE);
	}

	jpeg_start_decompress(&info);
	imgWidth = info.output_width;
	imgHeight = info.output_height;
	numComponents = info.num_components; // pixel size

	#ifdef DEBUG
		fprintf(
			stderr,
			"%s:%d: Reading JPEG with dimensions %lu x %lu and %d components\n",
			__FILE__, __LINE__,
			imgWidth, imgHeight, numComponents
		);
	#endif

	dwBufferBytes = imgWidth * imgHeight * 3; /* We only read RGB, not A */
	lpData = (unsigned char*)malloc(sizeof(unsigned char)*dwBufferBytes);

	lpNewImage = (struct imgRawImage*)malloc(sizeof(struct imgRawImage));
	lpNewImage->numComponents = numComponents;
	lpNewImage->width = imgWidth;
	lpNewImage->height = imgHeight;
	lpNewImage->dwBufferBytes = dwBufferBytes;
	lpNewImage->lpData = lpData;

	/* flip (mirror) the image along the horizontal axis:

	   Stephane Hockenhull: "OpenGL texture are loaded left to right, bottom to top.
	   Many image loaders however will store the image in memory left to right, top to bottom."
	*/
	unsigned long max_index_for_flip_image = info.output_width * 3 * (info.output_height - 1);
	// fixme: disable flip for jpeg output

	/* Read scanline by scanline */
	while(info.output_scanline < info.output_height) {
		lpRowBuffer[0] = (unsigned char *)(&lpData[max_index_for_flip_image    -    3*info.output_width*info.output_scanline]);
		jpeg_read_scanlines(&info, lpRowBuffer, 1);
	}

	jpeg_finish_decompress(&info);
	jpeg_destroy_decompress(&info);

	return lpNewImage;
}



int storeJpegImageFile(struct imgRawImage* lpImage, char* lpFilename) {
	struct jpeg_compress_struct info;
	struct jpeg_error_mgr err;

	unsigned char* lpRowBuffer[1];

	FILE* fHandle;

	fHandle = fopen(lpFilename, "wb");
	if(fHandle == NULL) {
		#ifdef DEBUG
			fprintf(stderr, "%s:%d Failed to open output file %s\n", __FILE__, __LINE__, lpFilename);
		#endif
		return 1;
	}

	info.err = jpeg_std_error(&err);
	jpeg_create_compress(&info);

	jpeg_stdio_dest(&info, fHandle);

	info.image_width = lpImage->width;
	info.image_height = lpImage->height;
	info.input_components = 3;
	info.in_color_space = JCS_RGB;

	jpeg_set_defaults(&info);
	jpeg_set_quality(&info, 100, TRUE);

	jpeg_start_compress(&info, TRUE);

	/* Write every scanline ... */
	while(info.next_scanline < info.image_height) {
		lpRowBuffer[0] = &(lpImage->lpData[info.next_scanline * (lpImage->width * 3)]);
		jpeg_write_scanlines(&info, lpRowBuffer, 1);
	}

	jpeg_finish_compress(&info);
	fclose(fHandle);

	jpeg_destroy_compress(&info);
	return 0;
}



void process_image(AVFrame *pFrameRGB, int frame_count, int compare_with_first, int verbose, unsigned int video_texture, int num_components, OPTICAL_FLOW* flow)
{
	extern struct imgRawImage* raw_image; // fixme: global variable
	extern struct imgRawImage* gui_image; // fixme: global variable
	extern struct imgRawImage* old_image; // fixme: global variable
	//extern OPTICAL_FLOW* flow; // fixem: global variable

	raw_image = (struct imgRawImage*)malloc(sizeof(struct imgRawImage));
	raw_image->numComponents = num_components;
	raw_image->width = pFrameRGB->width;
	raw_image->height = pFrameRGB->height;
	raw_image->dwBufferBytes = pFrameRGB->width * pFrameRGB->height * num_components;
	raw_image->lpData = (unsigned char*)malloc(sizeof(unsigned char) * (raw_image->dwBufferBytes));

	// memcpy not work because uint8_t != unsigned char:
	// memcpy(lpData, pFrameRGB->data, sizeof(unsigned char) * raw_image->dwBufferBytes);
	//
	// also need flip image by horizontal axis:
	for (int j=0; j < pFrameRGB->height; j++) {
		for (int i=0; i < pFrameRGB->linesize[0]; i++) {
			raw_image->lpData         [i + (pFrameRGB->height - j - 1) * pFrameRGB->linesize[0]] =
				pFrameRGB->data[0][i +                      j      * pFrameRGB->linesize[0]];
		}
	}

	if (verbose != VERBOSE_NO) {
		gui_image = (struct imgRawImage*)malloc(sizeof(struct imgRawImage));
		gui_image->numComponents = raw_image->numComponents;
		gui_image->width         = raw_image->width;
		gui_image->height        = raw_image->height;
		gui_image->dwBufferBytes = raw_image->dwBufferBytes;
		gui_image->lpData = (unsigned char*)malloc(sizeof(unsigned char) * (gui_image->dwBufferBytes));
		memcpy(gui_image->lpData, raw_image->lpData, sizeof(unsigned char) * gui_image->dwBufferBytes);
	}

	if (old_image != NULL) {
		//block_matching_full_images (old_image, raw_image, gui_image, MAX_SHIFT, BLOCK_SIZE); // optical flow
		block_matching_optimized_images (old_image, raw_image, gui_image, flow); // optical flow
	}

	

	if (verbose & VERBOSE_IMAGE) {
		draw_crosshair(gui_image);
		// save frame as a JPEG file
		char file_name[MAX_FNAME_LEN];
		sprintf(file_name, "/tmp/image_%04d.jpeg", frame_count);
		int ret;
		ret = storeJpegImageFile(gui_image, file_name);
		if (ret != 0) printf("error store jpeg file");
	}

	if (verbose & VERBOSE_VIDEO) {
		render_loop (gui_image, video_texture);
	}



	if (verbose != VERBOSE_NO) {
		free(gui_image->lpData);
		free(gui_image);
	}

	if (old_image != NULL && compare_with_first != true) {
		free(old_image->lpData);
		free(old_image);
	}

	if (compare_with_first == false ||
	    (compare_with_first == true && frame_count == 1)) {
		old_image = raw_image;
		old_image->lpData = raw_image->lpData;
	}

	fflush(stderr);
	fprintf(stderr, ".%s", (frame_count % 100 == 0)? "\n" : "");
	fflush(stdout);
}

long long int coord_to_raw_chunk(struct imgRawImage* image, COORD_2DU coord)
{
	if (coord.x >= image->width ||
	    coord.y >= image->height) return -1;
	long long int raw_chunk = (coord.y * image->width + coord.x) * image->numComponents;
	if (raw_chunk > (long long int)image->dwBufferBytes) return -1;
	return raw_chunk;
}

struct coord_2Du raw_chunk_to_coord(struct imgRawImage* image, unsigned long int r)
{
	struct coord_2Du c;
	unsigned long int wn = image->width * image->numComponents; // fixme: wn==constant and need to calculate only once
	c.y = r / wn;
	//c.x = (r % wn) / image->numComponents;
	c.x = (r - c.y * wn) / image->numComponents;
	return c;
}
