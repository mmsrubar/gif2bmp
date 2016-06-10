/*
 * Autor: Michal Šrubař
 * Email: xsruba03@stud.fit.vutbr.cz
 * Date: 	Sat Mar  5 15:26:53 CET 2016
 * Modul: GIF Interlacing
 *
 * This module reads uncompressed image data and uses color table to write RGB
 * values into a file. It's used for testing when we output rgb colors file out
 * of a GIF image and compare it with output of the 'gif2rgb' utility.
 *
 */

#include "gif_rgb.h"
#include "gif_lzw.h"
#include <string.h>
#include <stdlib.h>

int gif_save_rgb(FILE *in, FILE *out)
{
	tGIF *img;

	// load image into memory
	if ((img = gif_load(in)) == NULL)
		return 1;

	// decompress the image data
	if (gif_lzw_decompression(img) != 0)
		return 1;

	/*
	 * Each index represents index, to the color table of one pixel of the image
	 * from left top corner to the right bottom corner.
	 */
	for (int i = 0; i < img->data.size; i++)
	{
		fwrite(&(img->colors_tab[img->data.indexes[i]].r), sizeof(u_int8_t), 1, out);
		fwrite(&(img->colors_tab[img->data.indexes[i]].g), sizeof(u_int8_t), 1, out);
		fwrite(&(img->colors_tab[img->data.indexes[i]].b), sizeof(u_int8_t), 1, out);
	}

	gif_unload(img);
	return 0;
}
