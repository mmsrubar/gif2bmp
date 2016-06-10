#ifndef BMP_H
#define BMP_H

#include "gif.h"
#include <stdio.h>
#include <sys/types.h>

#define B		0x42
#define M		0x4d
#define PLANES	0x01
#define BI_RGB	0
#define BI_RLE8	1
#define BI_RLE4	2

#define FILEHEADER_SIZE		14
#define INFOHEADER_SIZE		0x28	// 40 bytes

/*
 * Write general information about the bitmap image file.
 *
 * @param file Pointer to the output bitmap file
 * @param bfOffBits Offset of the actual data
 */
void bmp_add_fileHeader(FILE *file, u_int32_t bfOffBits);

/*
 * To store detailed information about the bitmap image and define the pixel
 * format.
 *
 * @param file Pointer to the output bitmap file
 * @param img Gif image
 * @param color_tab_size Length of the color table
 *
 */
u_int16_t bmp_add_infoHeader(FILE *file, tGIF *img, int color_tab_size);

/*
 * Save the color table.
 *
 * @param file Pointer to the output bitmap file
 * @param tab Pointer to the color table
 * @param tab_len Length of the color table
 */
void bmp_add_rgbQuad(FILE *file, struct color *tab, int tab_len);

/* 
 * Save the actual uncompressed data.
 *
 * @param file Pointer to the output bitmap file
 * @param img Gif image
 *
 */
void bmp_add_bits(FILE *file, tGIF *img, u_int16_t biBitCount);

/*
 * Correct the total size.
 */
u_int32_t bmp_add_bfSize(FILE *outputFile);

#endif	// BMP_H
