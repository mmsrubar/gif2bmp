/*
 * Autor: Michal Šrubař
 * Email: xsruba03@stud.fit.vutbr.cz
 * Date: 	Sat Mar  5 15:26:53 CET 2016
 * Modul: Gif Image
 *
 * This module performs the loading of a GIF image into the tGIF structure.
 *
 */

#include "gif.h"
#include "gif_bits.h"
#include <string.h>
#include <stdlib.h>
#include <string.h>

#define GLOB_COLOR_TAB_FLAG_POS	7
#define LOGICAL_DESC_SORT_FLAG	3

/* Extensions are not supported yet */
#define EXT_ERROR_MSG(e) fprintf(stderr, "Image contains %s Extension which is not suppoerted\n", e)
/* Skip extension e by reading n bytes in file f and then read another byte b */
#define SKIP_EXTENSION(f, e, n, b) \
	EXT_ERROR_MSG(e); \
	fseek(f, n, SEEK_CUR); \
	b = fgetc(f);	\

/* 
 * Read 2 bytes from the current possition from the file and convert it to a 16
 * bit number.
 */
static u_int16_t read_word(FILE *f)
{
	u_int8_t byte = fgetc(f);
	u_int8_t byte2 = fgetc(f);
	return 256 * (byte2) + byte;	// little endian ad01 = ad + 256*01
}

/*
 * Load either global or local color table.
 * @param glob	true if we are about to load the global color table
 */
static void gif_load_color_table(tGIF *img, FILE *f, int size)
{
	for (int i = 0; i < size; i++) {
		img->colors_tab[i].r = fgetc(f);
		img->colors_tab[i].g = fgetc(f);
		img->colors_tab[i].b = fgetc(f);
		img->size += COLOR_SIZE;
	}
}

/* Load header and test if the image has the corrent signature and version */
static int gif_load_header(tGIF *img, FILE *f)
{
	fgets((char *) img->signature, 4, f);
	fgets((char *) img->version, 4, f);

	if (! strncmp((const char *) img->signature, SIGNATURE, 4) == 0) {
		fprintf(stderr, "The image file doesn't have the GIF signature\n");
		return 1;
	}
	if (! strncmp((const char *) img->version, VERSION1, 4) == 0 && 
			! strncmp((const char *) img->version, VERSION2, 4) == 0) {
		fprintf(stderr, "The image file doesn't have the GIF signature\n");
		fprintf(stderr, "Supported GIF versions are 89a and 87a\n");
		return 1;
	}

	img->size += HEADER_SIZE;

	return 0;
}

/* Load the Logical Screen Descriptor information */
static int gif_load_lsd(tGIF *img, FILE *f)
{
	u_int8_t bit_field;
	u_int8_t bits;

	struct logical_screen_descriptor *lsd = &(img->screen_desc);

	lsd->width = read_word(f);
	lsd->height = read_word(f);
	bit_field = fgetc(f);
	lsd->glob_colors = GET_BIT(bit_field, GLOB_COLOR_TAB_FLAG_POS);
	bits = bit_field & 0x70; // get value of 6.-4. bit
	lsd->color_resolution = (int)(bits>>4);
	lsd->sort = GET_BIT(bit_field, LOGICAL_DESC_SORT_FLAG);
	lsd->glob_colors_len = bit_field & 0x7;
	lsd->glob_colors_len_real = POW2(lsd->glob_colors_len+1);
	lsd->background_color = fgetc(f);
	lsd->pixel_aspect_ratio = fgetc(f);

	img->size += LOCAL_SCREEN_DESC_SIZE;

	return 0;
}

/* Load Image Descriptor */
static int gif_load_img_desc(tGIF *img, FILE *f, u_int8_t byte)
{
	u_int8_t bit_field;
	struct image_descriptor *id = &(img->image_desc);

	if (byte == IMAGE_SEPARATOR) {
		id->image_left = read_word(f);
		id->image_top = read_word(f);
		id->image_width = read_word(f);
		id->image_height = read_word(f);
		bit_field = fgetc(f);
		id->local_color_table_flag = (GET_BIT(bit_field, 7) == 128) ? true : false;
		id->local_colors_len = bit_field & 0x7;
		id->local_colors_len_real = POW2(id->local_colors_len+1);
		id->interlance = (GET_BIT(bit_field, 6) == 64) ? true : false;
		id->sort = (GET_BIT(bit_field, 5) == 32) ? true : false;
	}
	else {
		fprintf(stderr, "Image descriptor should follow\n");
		return 1;
	}

	img->size += IMAGE_DESC_SIZE;

	return 0;
}

/* 
 * After the global color table or image data there can be:
 * - Application Extension or
 * - Comment Extension or
 * - Graphics Control Extension or
 * - Plain Text Extension.
 * Then the image descriptor has to follow.
 *
 * FIXME: Handle situations where there more extensions in row!!_
 */
static void gif_load_extensions(tGIF *img, FILE *f, u_int8_t *byte)
{
	while (*byte == EXTENSION)
	{
		switch (fgetc(f)) {
			case APP_EXTENSION_LABEL:
				// -2 because we already read the first 2 bytes
				SKIP_EXTENSION(f, "Application", APP_EXT_SIZE-2, *byte);
				fprintf(stderr, "Animated gifs aren't supported\n");
				img->size += APP_EXT_SIZE;
				break;
			case COMMENT_LABEL:
				img->size += 2;
				while (fgetc(f) != 0x00)	// 0x00 is a comment terminator
					img->size += 1;	
				// read another byte so while can catch if there is another extenssion following
				*byte = fgetc(f); img->size += 1;
				break;
			case PLAIN_TEXT_LABEL:
				SKIP_EXTENSION(f, "Plain Text", PLAIN_TEXT_EXT_SIZE-2, *byte);
				img->size += PLAIN_TEXT_EXT_SIZE;
				break;
			case GRAPHICS_CTRL_LABEL:
				SKIP_EXTENSION(f, "Graphics Control", GRAPHICS_CTRL_EXT_SIZE-2, *byte);	
				img->size += GRAPHICS_CTRL_EXT_SIZE;
				break;
		}
	}
}

/* 
 * Load entire GIF image into the memory. The internal structures of a gif file
 * are then represented by the tGIF structure.
 */
tGIF *gif_load(FILE *f)
{
	tGIF *img;

 	if ((img	= (tGIF *) malloc(sizeof(tGIF))) == NULL) {
		fprintf(stderr, "Error while allocating memory\n");
		return NULL;
	}

	img->data.indexes = NULL;
	img->data_lzw.data = NULL;
	img->size = 0;

	// Load the Header
	if (gif_load_header(img, f) != 0)
		return NULL;

	// Load the Logical Screen Descriptor
	if ((gif_load_lsd(img, f)) != 0)
		return NULL;

	// Load global color table (if any)
	if (img->screen_desc.glob_colors)
		gif_load_color_table(img, f, img->screen_desc.glob_colors_len_real);

	u_int8_t next_byte = fgetc(f);
	gif_load_extensions(img, f, &next_byte);

	if ((gif_load_img_desc(img, f, next_byte)) != 0)
		return NULL;

	// Local color table (if any)
	if (img->image_desc.local_color_table_flag)
		gif_load_color_table(img, f, img->image_desc.local_colors_len_real);

	// FIXME: for now we only support one image
	// Image data
	img->lzw_min_code_size = fgetc(f);
	img->size += 1;

	// initialize linked list for data compressed with LZW
	INIT_LIST_HEAD(&img->data_lzw.list);

	struct sub_block *block;
	u_int8_t tmp_size = fgetc(f);

	while (tmp_size != 0x00)
	{
		// create an item for a new sub-block
		if ((block = (struct sub_block *)malloc(sizeof(struct sub_block))) == NULL) {
			fprintf(stderr, "Error while allocating memory\n");
			return NULL;
		}

		block->size = tmp_size;
		img->size += tmp_size + 1;		// +1 for the block size itself

#ifdef DEBUG
		printf("block size: %d\n", block->size);
#endif
	
		// create an array for bytes of the sub-block
		if ((block->data = (u_int8_t *)malloc(sizeof(u_int8_t) * block->size)) == NULL) {
			fprintf(stderr, "Error while allocating memory\n");
			return NULL;
		}

		// read all bytes from the sub-block
		// FIXME: can't we just use read to read all bytes at once?
		for (int i = 0; i < block->size; i++)
		{
			block->data[i] = fgetc(f);
#ifdef DEBUG
			printf("data[%d]: %d\n", i, block->data[i]);
#endif
		}

  	list_add_tail(&(block->list), &(img->data_lzw.list));
		tmp_size = fgetc(f);
#ifdef DEBUG
		printf("next size: %x\n",  tmp_size);
#endif
	}

	img->size += 1;		// block terminator (00)

	next_byte = fgetc(f);
#ifdef DEBUG
		printf("byte after lzw data: %x\n", next_byte);
#endif
	gif_load_extensions(img, f, &next_byte);

	if (next_byte == TRAILER)
	{
		img->size += 1;
		return img;
	}
	else {
		fprintf(stderr, "Error: GIF file is not correctly ended with 0x%x\n", TRAILER);
		return NULL;
	}
}

/* 
 * Free entire GIF image from memory.
 */
void gif_unload(tGIF *img)
{
	struct sub_block *block;
	struct list_head *pos, *q;

	if (img == NULL)
		return;

	list_for_each_safe(pos, q, &img->data_lzw.list){
		block = list_entry(pos, struct sub_block, list);
		list_del(pos);  		// 1st remove sub block from the list
		free(block->data);	// then sub-block's data
		free(block);     		// then sub-block itself
	}

	free(img->data.indexes);
	free(img);
}
