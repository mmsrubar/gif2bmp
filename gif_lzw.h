#ifndef GIF_LZW_H
#define GIF_LZW_H

#include "gif.h"

/* 
 * Take compressed data saved in img->data_lzw and use LZW algorithm to
 * uncompress them. The uncompressed data which are indexes to the color table
 * are then saved into the img->data->indexes array.
 *
 * @param img Pointer to the GIF image structure
 *
 * @return 0 On success
 * @return 1 On failure
 */
int gif_lzw_decompression(tGIF *img);

#endif	// GIF_LZW_H
