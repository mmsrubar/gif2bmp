/*
 * Autor: Michal Šrubař
 * Email: xsruba03@stud.fit.vutbr.cz
 * Date: 	Sat Mar  5 15:26:53 CET 2016
 * Modul: GIF Interlacing
 *
 * This module is used to converet interlaced lines into sequencial order when
 * the GIF data are interlaced.
 *
 */

#include "gif_interlance.h"
#include <string.h>
#include <stdlib.h>

/*
 * If the interlacing is on then the rows of the GIF image are not in sequence
 * order. 
 *
 * The rows of an Interlaced images are arranged in the following order:
 * Every 8th. row, starting with row 0.              (Pass 1)
 * Every 8th. row, starting with row 4.              (Pass 2)
 * Every 4th. row, starting with row 2.              (Pass 3)
 * Every 2nd. row, starting with row 1.              (Pass 4)
 *
 * An example:
 * Let's suppose that we have uncompressed data of an image which is 23 pixels
 * wide and 23 pixels high. The data of the image could look like:
 *
 * 0: i01 i02 i03 ... i023
 * 1: i11 i12 i13 ... i123
 * .
 * .
 * .
 * 23:i231 i232 i233 ... i2323
 *
 * where numbers iNN represent index to a color table of an index. For example
 * i01 would be index of a color of the left-top pixel. The data can be
 * represented as 23 lines and each line has 23 indexes.
 *
 * With interlacing off the image's data would be stored line by line from zero to
 * twenty-trhird line. But with interlacing on the order won't sequential
 * anymore.
 *
 * 				 line		interlanced
 * ---------------------------------------
 *           0         0      Pass1
 *           1         8      Pass1
 *           2        16      Pass1
 *           3         4      Pass2
 * strip 1   4        12      Pass2
 *           5        20      Pass2
 *           6         2      Pass3 line 2
 *           7         6      Pass3 line 2
 * -----------        
 *           8        10      Pass3 line 2
 *           9        14      Pass3 line 6
 *          10        18      Pass3 line 6
 *          11        22      Pass3 line 6 
 * strip 2  12         1      Pass4 line 1
 *          13         3      Pass4 line 1
 *          14         5      Pass4 line 1 
 *          15         7      Pass4 line 3
 * -----------        
 *          16         9      Pass4 line 3
 *          17        11      Pass4 line 3  
 *          18        13      Pass4 line 5
 *          19        15      Pass4 line 5
 * strip 3  20        17      Pass4 line 5 
 *          21        19      Pass4 line 7 
 *          22        21      Pass4 line 7 
 *          23        23      Pass4 line 7 
 * ---------------------------------------
 *
 * This is how input lines are interlanced. During the decompression we have to
 * get sequence lines from interlanced lines.
 *
 */
int gif_lzw_interlacing(tGIF *img)
{
	u_int16_t height = img->image_desc.image_height;
	u_int16_t width = img->image_desc.image_width;

	// we will be saving lines to the new array from 0 to size of the old array
	int offset_new = 0;
	int offset_old = 0;

	// create space for non-interlaced data
	u_int8_t *new_indexes = (u_int8_t *) malloc(sizeof(u_int8_t) * img->data.size);
	if (new_indexes == NULL)
		handle_error("Error while allocating memory", 1);

	// copy every 8th, 4th, 2nd, 6th, ... line
	int lines[] = {0,4,2,1};
	int step[] = {8,8,4,2};

	for (unsigned int i = 0; i < sizeof(lines)/sizeof(int); i++)
	{
		offset_new = lines[i] * width;

		while (offset_new < width * height)
		{
			// copy a single line
			memcpy(new_indexes + offset_new, img->data.indexes + offset_old, width);
			// move one line down in the new array
			offset_old += width;
			// move to another 8-th line in the old array
			offset_new += step[i] * width;
		}
	}

	// free the interlaced data first and then save the pointer to new ones
	free(img->data.indexes);
	img->data.indexes = new_indexes;

	return 0;
}
