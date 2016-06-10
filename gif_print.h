#ifndef GIF_PRINT_H
#define GIF_PRINT_H

#include "gif.h"

/* 
 * Print information about a GIF image 
 *
 * @param in Pointer to an opened GIF image
 *
 * @return 0 On success
 * @return 1 On failure
 */
int gif_print_info(FILE *in);

#endif	// GIF_PRINT_H
