/** \file
   image-type.h --- header for image.c

   Copyright (C) 2021 Roman V. Prikhodchenko

   Author: Roman V. Prikhodchenko <chujoii@gmail.com>
*/

// include guard
#ifndef IMAGE_TYPE_H
#define IMAGE_TYPE_H

struct coord_2Du {
	unsigned long int x;
	unsigned long int y;
};

struct imgRawImage {
	unsigned int numComponents;
	unsigned long int width, height;
	unsigned long int dwBufferBytes; // = width * height * numComponents;
	unsigned char* lpData;
};


enum colors {R, G, B};

#endif /* IMAGE_TYPE_H */
