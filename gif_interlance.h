#ifndef GIF_INTERLANCE_H
#define GIF_INTERLANCE_H

#include "gif.h"

/*
 * Convert interlaced image into non interlaced one. This is automatically done
 * by the gif_lzw_decompression function.
 *
 * @param img Pointer to the GIF image structure
 *
 * @return 0 On success
 * @return 1 On failure
 */
int gif_lzw_interlacing(tGIF *img);

#endif	// GIF_INTERLANCE_H
