/** \file
   image.h --- header for image.c

   Copyright (C) 2021 Roman V. Prikhodchenko

   Author: Roman V. Prikhodchenko <chujoii@gmail.com>
*/

// include guard
#ifndef IMAGE_H
#define IMAGE_H

#include <libavutil/frame.h>

#include "image-type.h"
#include "block-matching-type.h"

#define NUM_COMPONENTS_RGB 3

struct imgRawImage* loadJpegImage(const void *jpg_buffer, int jpg_size);
int storeJpegImageFile(struct imgRawImage* lpImage, char* lpFilename);
void process_image(AVFrame *pFrameRGB, int frame_count, int verbose, unsigned int video_texture, int num_components, OPTICAL_FLOW* flow);
long long int coord_to_raw_chunk(struct imgRawImage* image, COORD_2DU coord);
struct coord_2Du raw_chunk_to_coord(struct imgRawImage* image, unsigned long int r);

#endif /* IMAGE_H */
