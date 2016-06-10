/*
 *
 * Autor: Michal Šrubař
 * Email: xsruba03@stud.fit.vutbr.cz
 * Date: 	Sat Mar  5 15:26:53 CET 2016
 * Modul: GIF to BMP
 *
 * This module implement function which performs the conversion of a GIF image
 * to bitmap image.
 *
 */
#include "gif2bmp.h"
#include "gif.h"
#include "gif_lzw.h"
#include "bmp.h"
#include "gif_bits.h"
#include <stdio.h>

int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile)
{
	u_int16_t biBitCount;
	tGIF *img;

	// load image into memory
	if ((img = gif_load(inputFile)) == NULL)
		return -1;

	// decompress the image's data
	if (gif_lzw_decompression(img) != 0)
		return -1;

	struct logical_screen_descriptor *lsd = &(img->screen_desc);
	struct image_descriptor *id = &(img->image_desc);
	int color_tab_size = (lsd->glob_colors) ? lsd->glob_colors_len : id->local_colors_len;
	u_int32_t bfOffBits = FILEHEADER_SIZE+INFOHEADER_SIZE+4*(POW2(color_tab_size+1));

	bmp_add_fileHeader(outputFile, bfOffBits);
	biBitCount = bmp_add_infoHeader(outputFile, img, color_tab_size);
	bmp_add_rgbQuad(outputFile, img->colors_tab, POW2(color_tab_size+1));
	bmp_add_bits(outputFile, img, biBitCount);

	gif2bmp->bmpSize = bmp_add_bfSize(outputFile);
	gif2bmp->gifSize = img->size;

	gif_unload(img);

	return 0;
}
