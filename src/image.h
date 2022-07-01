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


#define NUM_COMPONENTS 3 // RGB

struct imgRawImage* loadJpegImage(const void *jpg_buffer, int jpg_size);
int storeJpegImageFile(struct imgRawImage* lpImage, char* lpFilename);
void process_image(AVFrame *pFrameRGB, int frame_count, int verbose, unsigned int video_texture);
unsigned long int coord_to_raw_chunk(int image_width, struct coord_2Du coord);
struct coord_2Du raw_chunk_to_coord(int image_width, unsigned long int r);

#endif /* IMAGE_H */
