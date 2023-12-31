/** \file
   capture.h --- header for capture.c

   Copyright (C) 2021 Roman V. Prikhodchenko

   Author: Roman V. Prikhodchenko <chujoii@gmail.com>
*/

// include guard
#ifndef CAPTURE_H
#define CAPTURE_H

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>  // <-- Requiered for av_image_get_buffer_size
#include <libavutil/opt.h> // for av_opt_set
#include <libavdevice/avdevice.h>

#include "image-type.h"
#include "block-matching-type.h"

int mainloop(char *file_name, int max_frame_count, int compare_with_first, unsigned int video_texture);
void save_gray_frame(unsigned char *buf,int wrap,int xsize,int ysize, char *filename);
void save_rgb_frame(unsigned char* buf, int wrap, int xsize, int ysize, char* filename);
int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame,AVFrame *pFrameRGB,struct SwsContext *sws_ctx, int compare_with_first, unsigned int video_texture, int num_components, OPTICAL_FLOW* flow);



#endif /* CAPTURE_H */
