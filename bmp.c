/*
 *
 * Autor: Michal Šrubař
 * Email: xsruba03@stud.fit.vutbr.cz
 * Date: 	Sat Mar  5 15:26:53 CET 2016
 * Modul: BMP
 *
 * This module implement function which are used to map GIF internal structure
 * to BMP structure.
 *
 */
#include "bmp.h"
#include "gif_bits.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef DEBUG
	#define FWRITE(data, type, size, file) \
		printf("[0x%lx]: %d, 0x%x\n", ftell(file), *data, *data); \
		fwrite(data, sizeof(type), size, file);
#else
	#define FWRITE(data, type, size, file) fwrite(data, sizeof(type), size, file);
#endif

static u_int32_t bfSize = 0;

#define NIBBLE_FLUSH \
		fwrite(&byte, sizeof(u_int8_t), 1, file); \
		/*printf("byte: "); BYTE_TO_BIN_STR(byte); putchar('\n'); \*/ \
		high_nibble = true; \
		byte = 0x00; \
		(*bytes_written)++;
	
void bmp_add_fileHeader(FILE *file, u_int32_t bfOffBits)
{
	u_int8_t bfType[2];
	u_int16_t bfReserved = 0;

	bfType[0] = B;
	bfType[1] = M;

#ifdef DEBUG
	printf("Write File Header bytes\n");
#endif

	FWRITE(&bfType, u_int8_t, 2, file);
	FWRITE(&bfSize, u_int32_t, 1, file);	// we be corrected at the end
	FWRITE(&bfReserved, u_int16_t, 1, file);
	FWRITE(&bfReserved, u_int16_t, 1, file);
	FWRITE(&bfOffBits, u_int32_t, 1, file);
	
	bfSize += FILEHEADER_SIZE;
}

u_int16_t bmp_add_infoHeader(FILE *file, tGIF *img, int color_tab_size)
{
	u_int32_t biSize = INFOHEADER_SIZE;
	u_int32_t biWidth = img->image_desc.image_width;
	u_int32_t biHeight = img->image_desc.image_height;
	u_int16_t biPlanes = PLANES;
	u_int16_t biBitCount;
	u_int32_t biCompression = BI_RGB;
	u_int32_t biSizeImage = 0;
	u_int32_t biXPelsPerMeter = 0;
	u_int32_t biYPelsPerMeter = 0;
	u_int32_t biClrUsed = POW2(color_tab_size+1);
	u_int32_t biClrImportant = POW2(color_tab_size+1);
	int bits;

	/* There can be an image which has color resolution to set to 1 and using 256
	 * colors.
	 */
	if (color_tab_size > img->screen_desc.color_resolution)
		bits = color_tab_size;
	else 
		bits = img->screen_desc.color_resolution;

	switch (bits) {
		case 0:
			biBitCount = 1;
			break;
		case 1:
		case 2:
		case 3:
			biBitCount = 4;
			break;
		case 4:
		case 5:
		case 6:
		case 7:
			biBitCount = 8;
			break;
	}

#ifdef DEBUG
	printf("Write Info Header bytes\n");
#endif

	FWRITE(&biSize, u_int32_t, 1, file);
	FWRITE(&biWidth,u_int32_t, 1, file);
	FWRITE(&biHeight,u_int32_t, 1, file);
	FWRITE(&biPlanes, u_int16_t, 1, file);
	FWRITE(&biBitCount, u_int16_t, 1, file);
	FWRITE(&biCompression, u_int32_t, 1, file);
	FWRITE(&biSizeImage, u_int32_t, 1, file);
	FWRITE(&biXPelsPerMeter, u_int32_t, 1, file);
	FWRITE(&biYPelsPerMeter, u_int32_t, 1, file);
	FWRITE(&biClrUsed, u_int32_t, 1, file);
	FWRITE(&biClrImportant, u_int32_t, 1, file);

	bfSize += INFOHEADER_SIZE;

	return biBitCount;
}

void bmp_add_rgbQuad(FILE *file, struct color *tab, int tab_len)
{
	u_int8_t zero = 0x00;

#ifdef DEBUG
	printf("Write BRG Quad bytes\n");
#endif

	for (int i = 0; i < tab_len; i++) {
		// bmp uses BGR instead of RGB!
		FWRITE(&tab[i].b, u_int8_t, 1, file);
		FWRITE(&tab[i].g, u_int8_t, 1, file);
		FWRITE(&tab[i].r, u_int8_t, 1, file);
		FWRITE(&zero, u_int8_t, 1, file);

		bfSize += 4;
	}
}

/* Returns true if we are in the middle of the byte */
static bool write_nibble(FILE *file, u_int8_t index, int *bytes_written, bool flush)
{
	static u_int8_t byte = 0x00;
	static bool high_nibble = true;

	if (flush)
	{
		NIBBLE_FLUSH;
		return false;
	}

	if (high_nibble && index < 16)
	{
		WRITE_HIGH_NIBBLE(byte, index);
		high_nibble = false;
		return true;
	}
	else if (high_nibble == false && index < 16)
	{
		WRITE_LOW_NIBBLE(byte, index);
		NIBBLE_FLUSH;
		return false;
	}

	return true;
}

/*
 * This function first finds out how many padding bytes we need to add and then
 * adds them to the output stream.
 *
 * @param bytes_written	How many bytes was written already?
 * @param file The output stream
 *
 * @return The number of padding bytes written
 */
static int add_padding_bytes(FILE *file, int bytes_written)
{
	int padding = 0;

	if ((bytes_written % 4) != 0)
	{
		/* how many padding bytes do I need to add */
		padding = NUM_DIVISIBLE_BY_FOUR(bytes_written) - bytes_written;
		u_int8_t index = 0x00;

		for (int k = 0; k < padding; k++)
			FWRITE(&index, u_int8_t, 1, file);
	}

	return padding;
}

static int write_bit(FILE *file, int *bytes_written, u_int8_t bit, bool flush)
{
	static int bit_idx = 7;		// what bit I have to write data on
	static u_int8_t byte = 0x00;

	if (flush)
	{ 
		FWRITE(&byte, u_int8_t, 1, file);
		(*bytes_written)++;
		bit_idx = 7;
		byte = 0x00;
		return 7;
	}

	if (bit) BIT_SET(byte, bit_idx);

	/* If I write the 0th bit I have to put the byte to the output and start from
	 * 7th bit again */
	if ((bit_idx-1) == -1)
	{
		FWRITE(&byte, u_int8_t, 1, file);
		(*bytes_written)++;
		bit_idx = 7; 
		byte = 0x00;
	}
	else
		bit_idx--;

	return bit_idx;
}

/*
 * The index 2D array represents the image but every value of the array isn't a
 * color of a pixel but an index to the color table instead.
 *
 * With this method the bit rate of the final bitmap will be 1 bit per pixel
 * which means the we can store 8 indexes to color table (8 pixels) in a single
 * byte.
 *
 * An example:
 * Let's say that we have 3x3 gif image consisting of only two colors so our
 * colors table looks like this:
 * 		color[0] = #ffffff
 * 		color[1] = #ff00ff
 *
 * The index array holding indexes to the color table looks like this:
 * 0 1 0
 * 1 1 1
 * 1 1 1
 *
 * In BMP the first line of the GIF file will be the last line and has to
 * alligned to 4 bytes. So the final BMP data has to be set this way:
 * 11100000 00000000 00000000 00000000
 * 11100000 00000000 00000000 00000000
 * 01000000 00000000 00000000 00000000
 *
 */
static void bmp_add_bit(FILE *file, tGIF *img)
{
	u_int16_t width = img->image_desc.image_width;
	u_int16_t height = img->image_desc.image_height;
	u_int8_t *indexes = img->data.indexes;

	for (int i = 1; i <= height; i++)
	{
		// skip data to the last row which has to be written as the first row
		u_int8_t *row = indexes + ((height - i) * width);
		int bytes_written = 0;
		int bit = 7;

		for (int j = 0; j < width; j++)
			bit = write_bit(file, &bytes_written, row[j], false);

		/* if we are in the middle of a byte then we have to put it on output */
		if (bit != 7) write_bit(file, &bytes_written, 0, true);
	
		/* Every row has to have byte size divisible by four */
		int padding = add_padding_bytes(file, bytes_written);

		bfSize += bytes_written + padding;
	}
}

static void bmp_add_nibble(FILE *file, tGIF *img)
{
	u_int16_t width = img->image_desc.image_width;
	u_int16_t height = img->image_desc.image_height;
	u_int8_t *indexes = img->data.indexes;

	for (int i = 1; i <= height; i++)
	{
		// skip data to the last row which has to be written as the first row
		u_int8_t *row = indexes + ((height - i) * width);
		int bytes_written = 0;
		bool middle = false;	// are we in the middle of a byte?

		for (int j = 0; j < width; j++)
			middle = write_nibble(file, row[j], &bytes_written, false);

		if (middle) write_nibble(file, 0, &bytes_written, true);

		/* Every row has to have byte size divisible by four */
		int padding = add_padding_bytes(file, bytes_written);

		bfSize += bytes_written + padding;
	}
}

static void bmp_add_byte(FILE *file, tGIF *img)
{
	u_int16_t width = img->image_desc.image_width;
	u_int16_t height = img->image_desc.image_height;
	u_int8_t *indexes = img->data.indexes;

	for (int i = 1; i <= height; i++)
	{
		// skip data to the last row which has to be written as the first row
		u_int8_t *row = indexes + ((height - i) * width);

		for (int j = 0; j < width; j++)
		{
			fwrite(&row[j], sizeof(u_int8_t), 1, file);
#ifdef DEBUG
			printf("bmp byte (write on addr 0x%lx: %hhu\n", ftell(file), row[j]);
#endif
		}

		/* Every row has to have byte size divisible by four */
		int padding = add_padding_bytes(file, width);

		bfSize += width + padding;
	}
}

void bmp_add_bits(FILE *file, tGIF *img, u_int16_t biBitCount)
{
#ifdef DEBUG
	printf("Write data bytes\n");
#endif

	if (biBitCount == 1)
		bmp_add_bit(file, img);
	else if (biBitCount == 4)
		bmp_add_nibble(file, img);
	else
		bmp_add_byte(file, img);
}

u_int32_t bmp_add_bfSize(FILE *outputFile)
{
	/* get back to the begining of the file, skip the type write the total size */
	fseek(outputFile, 2, SEEK_SET);
	fwrite(&bfSize, sizeof(u_int32_t), 1, outputFile);
	return bfSize;
}
