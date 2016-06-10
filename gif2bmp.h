#ifndef GIF2BMP_H
#define GIF2BMP_H

#include "gif.h"

typedef struct {
	int64_t bmpSize;
	int64_t gifSize;
} tGIF2BMP;

/*
 * Convert GIF image to a bitmap image.
 *
 * @param inputFile Pointer to a GIF image
 * @param outputFile Pointer to a bitmap image
 *
 * @return 0 On success
 * @return 1 On failure
 */
int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile);

#endif	// GIF2BMP_H
