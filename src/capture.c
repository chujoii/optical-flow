/** \file
capture.c --- capture frame from ffmpeg

Copyright (C) 2021 Roman V. Prikhodchenko

Authors: Leandro Moreira,
         Roman V. Prikhodchenko <chujoii@gmail.com>


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



Keywords: capture ffmpeg

Usage:

History:
  based on:

  https://github.com/leandromoreira/ffmpeg-libav-tutorial
  Author: Leandro Moreira
  License: BSD-3-Clause License

  Format (Container) - a wrapper, providing sync, metadata and muxing for the streams.
  Stream - a continuous stream (audio or video) of data over time.
  Codec - defines how data are enCOded (from Frame to Packet) and DECoded (from Packet to Frame).
  Packet - are the data (kind of slices of the stream data) to be decoded as raw frames.
  Frame - a decoded raw frame (to be encoded or filtered).

  https://stackoverflow.com/questions/69442603/ffmpeg-convert-ycbcr-to-rgb-using-sws-scale
  Author: Rotem



  other doc:

  https://stackoverflow.com/questions/9251747/record-rtsp-stream-with-ffmpeg-libavformat
  Authors: chatoooo, Ryan Ayers

  https://stackoverflow.com/questions/10715170/receiving-rtsp-stream-using-ffmpeg-library
  Authors: technique, syntheticgio

  http://dranger.com/ffmpeg/
  Authors: Fabrice Bellard, and a tutorial by Martin Bohme.

  https://trac.ffmpeg.org/wiki/Using%20libav*
  https://github.com/FFmpeg/FFmpeg/tree/master/doc/examples
  https://wiki.multimedia.cx/index.php?title=Category:FFmpeg_Tutorials

Code:
*/



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "capture.h"
#include "gui.h"
#include "const.h"
#include "block-matching.h"


#define MAX_FNAME_LEN 128



int mainloop(char *file_name, int max_frame_count, int compare_with_first, unsigned int video_texture) {
	extern int escape_status;
	int result;

	printf("initializing all the containers, codecs and protocols.\n");
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
	av_register_all(); // av_register_all() deprecated since ffmpeg 4.0
#endif
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 10, 100)
	avcodec_register_all();
#endif

	// AVFormatContext holds the header information from the format (Container)
	// Allocating memory for this component
	// http://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
	AVFormatContext* pFormatContext = avformat_alloc_context();
	if(!pFormatContext) {
		printf("Error allocating memory for format context");
		return EXIT_FAILURE;
	}


	avdevice_register_all();
	avformat_network_init();

	// Open the file and read its header. The codecs are not opened.
	// The function arguments are:
	// AVFormatContext (the component we allocated memory for),
	// url (filename),
	// AVInputFormat (if you pass NULL it'll do the auto detect)
	// and AVDictionary (which are options to the demuxer)
	// http://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga31d601155e9035d5b0e7efedc894ee49
	if ((result = avformat_open_input(&pFormatContext, file_name, NULL, NULL)) != 0) {
		printf("ERROR could not open the file: [%d] %s\n", result, av_err2str(result));
		return EXIT_FAILURE;
	}

	// now we have access to some information about our file
	// since we read its header we can say what format (container) it's
	// and some other information related to the format itself.
	printf("format %s, duration %ld us, bit_rate %ld\n", pFormatContext->iformat->name, pFormatContext->duration, pFormatContext->bit_rate);

	printf("finding stream info from format\n");
	// read Packets from the Format to get stream information
	// this function populates pFormatContext->streams
	// (of size equals to pFormatContext->nb_streams)
	// the arguments are:
	// the AVFormatContext
	// and options contains options for codec corresponding to i-th stream.
	// On return each dictionary will be filled with options that were not found.
	// https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#gad42172e27cddafb81096939783b157bb
	if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
		return EXIT_FAILURE;
	}

	// Dump information about file onto standard error
	av_dump_format(pFormatContext, 0, file_name, 0);



	// the component that knows how to enCOde and DECode the stream
	// it's the codec (audio or video)
	// http://ffmpeg.org/doxygen/trunk/structAVCodec.html
	AVCodec *pCodec = NULL;

	// this component describes the properties of a codec used by the stream i
	// https://ffmpeg.org/doxygen/trunk/structAVCodecParameters.html
	AVCodecParameters *pCodecParameters = NULL;
	int video_stream_index = -1;

	// loop through all the streams: search video stream
	for (unsigned int i = 0; i < pFormatContext->nb_streams; i++) {
		AVCodecParameters *pLocalCodecParameters =  NULL;

		pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
		printf("AVStream->time_base before open coded %d/%d\n",
		       pFormatContext->streams[i]->time_base.num, pFormatContext->streams[i]->time_base.den);
		printf("AVStream->r_frame_rate before open coded %d/%d\n",
		       pFormatContext->streams[i]->r_frame_rate.num, pFormatContext->streams[i]->r_frame_rate.den);
		printf("AVStream->start_time %ld\n", pFormatContext->streams[i]->start_time);
		printf("AVStream->duration 0x%lx\n", pFormatContext->streams[i]->duration);

		printf("finding the proper decoder (CODEC)\n");

		AVCodec *pLocalCodec = NULL;

		// finds the registered decoder for a codec ID
		// https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga19a0ca553277f019dd5b0fec6e1f9dca
		pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

		if (pLocalCodec==NULL) {
			printf("ERROR unsupported codec!\n");
			return -1;
		}

		// when the stream is a video we store its index, codec parameters and codec
		if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
			if (video_stream_index == -1) {
				video_stream_index = i;
				pCodec = pLocalCodec;
				pCodecParameters = pLocalCodecParameters;
			}

			printf("Video Codec: resolution %d x %d\n",
			       pLocalCodecParameters->width, pLocalCodecParameters->height);
		} else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
			printf("Audio Codec: %d channels, sample rate %d\n",
			       pLocalCodecParameters->channels, pLocalCodecParameters->sample_rate);
		}

		// print its name, id and bitrate
		printf("\tCodec %s ID %d bit_rate %ld\n",
		       pLocalCodec->name, pLocalCodec->id, pLocalCodecParameters->bit_rate);
	}

	if(video_stream_index == -1) {
		printf("Error: File does not contain video stream");
		return -1;
	}

	// https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
	AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
	if (!pCodecContext) {
		printf("failed to allocated memory for AVCodecContext\n");
		return -1;
	}

	// Fill the codec context based on the values from the supplied codec parameters
	// https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
	if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
		printf("failed to copy codec params to codec context\n");
		return -1;
	}

	// Initialize the AVCodecContext to use the given AVCodec.
	// https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
	if (avcodec_open2(pCodecContext, pCodec, NULL) < 0) {
		printf("failed to open codec through avcodec_open2\n");
		return -1;
	}

	// https://ffmpeg.org/doxygen/trunk/structAVFrame.html
	AVFrame *pFrame = av_frame_alloc();
	if (!pFrame) {
		printf("failed to allocated memory for AVFrame\n");
		return -1;
	}
	// https://ffmpeg.org/doxygen/trunk/structAVPacket.html
	AVPacket *pPacket = av_packet_alloc();
	if (!pPacket) {
		printf("failed to allocated memory for AVPacket\n");
		return -1;
	}

	int response = 0;

	///////////////////////////////// start prepare to convert YCbCr to RGB format (YCbCr is often confused with the YUV) //////////////////////////////////////////

	struct SwsContext *sws_ctx;
	sws_ctx = sws_getContext
		(
			pCodecContext->width,
			pCodecContext->height,
			pCodecContext->pix_fmt,
			pCodecContext->width,
			pCodecContext->height,
			AV_PIX_FMT_RGB24,
			SWS_BILINEAR,
			NULL,
			NULL,
			NULL
			);


	AVFrame *pFrameRGB = av_frame_alloc();
	int num_components = NUM_COMPONENTS_RGB; // fixme: can ffmpeg decode monochrome video (-pix_fmt gray)?

	//int num_bytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecContext->width, pCodecContext->height); //https://stackoverflow.com/questions/12831761/how-to-resize-a-picture-using-ffmpegs-sws-scale
	//avpicture_fill((AVPicture*)pFrameRGB, frame_buffer_RGB, AV_PIX_FMT_RGB24, pCodecContext->width, pCodecContext->height);  //deprecated use av_image_fill_arrays() instead.

	int num_bytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecContext->width, pCodecContext->height, 1); //https://stackoverflow.com/questions/35678041/what-is-linesize-alignment-meaning

	unsigned char* frame_buffer_RGB = (uint8_t*)av_malloc(num_bytes);

	response = av_image_fill_arrays(pFrameRGB->data,       //uint8_t *dst_data[4],
					pFrameRGB->linesize,   //int dst_linesize[4],
					frame_buffer_RGB,      //const uint8_t * src,
					AV_PIX_FMT_RGB24,      //enum AVPixelFormat pix_fmt,
					pCodecContext->width,  //int width,
					pCodecContext->height, //int height,
					1);                    //int align);

	if (response < 0) {
		printf("av_image_fill_arrays Failed, response = %d\n", response);
	}

	pFrameRGB->width = pCodecContext->width;
	pFrameRGB->height = pCodecContext->height;

	///////////////////////////////// end prepare to convert YCbCr to RGB format (YCbCr is often confused with the YUV) //////////////////////////////////////////

	OPTICAL_FLOW flow;
	init_block_matching (pFrameRGB->width, pFrameRGB->height, BLOCK_SIZE, MAX_SHIFT_GLOBAL, MAX_SHIFT_LOCAL, NANOSECONDS_IN_SECOND / FPS, &flow);

	// fill the Packet with data from the Stream
	// https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga4fdb3084415a82e3810de6ee60e46a61
	int ret;
	int frame_counter = 0;
	while ((ret = av_read_frame(pFormatContext, pPacket)) >= 0 &&
	       max_frame_count != 0 && escape_status == false) { // max_frame_count == -1 infinity; > 0 limited frame number; == 0 exit
		// if it's the video stream
		if (pPacket->stream_index == video_stream_index) {
			//printf("AVPacket->pts %ld\n", pPacket->pts);
			response = decode_packet(pPacket, pCodecContext, pFrame, pFrameRGB, sws_ctx, compare_with_first, video_texture, num_components, &flow);

			if (response < 0)
				break;
		}
		// https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html#ga63d5a489b419bd5d45cfd09091cbcbc2
		av_packet_unref(pPacket);
		printf(" %d=", frame_counter);
		if (max_frame_count > 0) max_frame_count--;
		frame_counter++;
	}

	printf("releasing all the resources\n");

	free_block_matching (&flow);
	avformat_close_input(&pFormatContext);
	av_free(frame_buffer_RGB);
	av_frame_free(&pFrameRGB);
	av_packet_free(&pPacket);
	av_frame_free(&pFrame);
	avcodec_free_context(&pCodecContext);
	return 0;
}


int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame, AVFrame *pFrameRGB, struct SwsContext *sws_ctx, int compare_with_first, unsigned int video_texture, int num_components, OPTICAL_FLOW* flow)
{
	extern int verbose;

	// Supply raw packet data as input to a decoder
	// https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga58bc4bf1e0ac59e27362597e467efff3
	int response = avcodec_send_packet(pCodecContext, pPacket);

	if (response < 0) {
		printf("Error while sending a packet to the decoder: %s\n", av_err2str(response));
		return response;
	}

	while (response >= 0) {
		// Return decoded output data (into a frame) from a decoder in YCbCr format (YCbCr is often confused with the YUV)
		// https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c
		response = avcodec_receive_frame(pCodecContext, pFrame);





		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
			break;
		} else if (response < 0) {
			printf("Error while receiving a frame from the decoder: %s\n", av_err2str(response));
			return response;
		}

		if (response >= 0) {
			/*
			printf(
				"Frame %d (type=%c, size=%d bytes) pts %ld key_frame %d [DTS %d]\n",
				pCodecContext->frame_number,
				av_get_picture_type_char(pFrame->pict_type),
				pFrame->pkt_size,
				pFrame->pts,
				pFrame->key_frame,
				pFrame->coded_picture_number
				);
			*/

			response = sws_scale(sws_ctx, (unsigned char const * const *)(pFrame->data), (pFrame->linesize),
					     0, pCodecContext->height, pFrameRGB->data, pFrameRGB->linesize);

			if (response <= 0) {
				printf("Error: sws_scale status = %d\n", response);
			}
			process_image(pFrameRGB, pCodecContext->frame_number, compare_with_first, verbose, video_texture, num_components, flow);
			if (verbose & VERBOSE_IMAGE) {
				char frame_filename[MAX_FNAME_LEN];
				/*
				// save a grayscale frame into a .pgm file
				snprintf(frame_filename, sizeof(frame_filename), "/tmp/%s-%d.pgm", "frame", pCodecContext->frame_number);
				if(pFrame->format != AV_PIX_FMT_YUV420P) {
					printf("Maybe not grayscale generated");
				}
				save_gray_frame(pFrame->data[0], pFrame->linesize[0], pFrame->width, pFrame->height, frame_filename);
				*/
				//Write RGB output to PPM image file
				snprintf(frame_filename, sizeof(frame_filename), "/tmp/%s-%d.ppm","frame", pCodecContext->frame_number);
				save_rgb_frame(pFrameRGB->data[0], pFrameRGB->linesize[0], pFrameRGB->width, pFrameRGB->height, frame_filename);
			}
		}
	}
	return 0;
}



void save_gray_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename)
{
	FILE *f;
	int i;
	f = fopen(filename,"w");
	// writing the minimal required header for a pgm file format
	// portable graymap format -> https://en.wikipedia.org/wiki/Netpbm_format#PGM_example
	fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

	// writing line by line
	for (i = 0; i < ysize; i++)
		fwrite(buf + i * wrap, 1, xsize, f);
	fclose(f);
}


void save_rgb_frame(unsigned char *buf, int wrap, int xsize, int ysize, char *filename)
{
	FILE* f;
	int i;
	f = fopen(filename, "wb");
	fprintf(f, "P6\n%d %d\n255\n", xsize, ysize);

	for (i = 0; i < ysize; i++) {
		unsigned char* ch = (buf + i * wrap);
		//ProcessArray(ch, xsize);
		fwrite(ch, 1, xsize*3, f);  //Write 3 bytes per pixel.
	}

	fclose(f);
}
