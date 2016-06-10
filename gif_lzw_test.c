#include <stdio.h>
#include "gif.h"
#include "gif_lzw.h"

int main(int argc, char *argv[])
{

	FILE  *f = fopen(argv[1], "rb");
	tGIF *img;

	// load image into memory
	if ((img = gif_load(f)) == NULL)
		return 1;
	// image is in the memory so we can close the file now
	fclose(f);

	gif_lzw_decompression(img);

	gif_unload(img);


	return 0;
}
