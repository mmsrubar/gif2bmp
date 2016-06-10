#ifndef GIF_RGB_H
#define GIF_RGB_H

#include "gif.h"

/*
 * Read GIF image from a file, decompress its data and then save RGB value of
 * each file into an output file.
 *
 * @param in Pointer to a GIF image
 * @param out Pointer to output file
 *
 * @return 0 On success
 * @return 1 On failure
 */
int gif_save_rgb(FILE *in, FILE *out);

#endif	// GIF_RGB_H
