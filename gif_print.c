/*
 * Autor: Michal Šrubař
 * Email: xsruba03@stud.fit.vutbr.cz
 * Date: 	Sat Mar  5 15:26:53 CET 2016
 * Modul: Show GIF structure
 *
 *
 * This module impplements print function which can print internal structure of
 * a GIF image. Mostly used for testing purposes.
 *
 */

#include "gif_print.h"

/* Get string representation of true or false value */
#define GET_T_OR_F(flag) ((flag) ? "true" : "false")

/* Print byte as a string of single bits */
static void byte_bin_str(u_int8_t byte)
{
	u_int8_t mask = 0x80;

	for (int i = 0; i < 8; i++)
		if (byte & (mask >> i)) putchar('1'); else putchar ('0');
}

/* print basic information */
static void gif_print_file_info(tGIF *img)
{
	printf("GIF Image:\n==========\n");
	printf("|- Size: %d\n", img->size);
}

/* print header (signature and version) */
static void gif_print_header(tGIF *img)
{
	printf("Header:\n=======\n");
	printf("|- signature: %s\n", img->signature);
	printf("|- version: %s\n", img->version);
}

static void gif_print_lsd(struct logical_screen_descriptor *lsd)
{
	printf("\nLogical Screen Descriptor:\n==========================\n");
	printf("|- width: %d\n", lsd->width);
	printf("|- height: %d\n", lsd->height);
	printf("|- global color table: %s\n", GET_T_OR_F(lsd->glob_colors));
	printf("|- color resolution: %d (+1 bits per pixel)\n", lsd->color_resolution + 1);
	printf("|- sort flag: %s\n", GET_T_OR_F(lsd->sort)); 
	if (lsd->glob_colors)
	{
		printf("|- glob. color tab. size: %d\n", lsd->glob_colors_len);
		printf("|- glob. color tab. size (real): %d colors\n", lsd->glob_colors_len_real);
	}
	printf("|- background color index: %x\n", lsd->background_color);
	printf("|- Pixel aspect ration: %x\n", lsd->pixel_aspect_ratio);
}

static void gif_print_color_table(tGIF *img, bool glob_tab)
{
	struct logical_screen_descriptor *sd = &(img->screen_desc);
	struct image_descriptor *id = &(img->image_desc);
	int len;

	/* If we want global color table but global color table flag is not set then
	 * there is nothing to print. Also for local color table.
	 */
	if ((glob_tab && sd->glob_colors == false) || (! glob_tab && sd->glob_colors))
		return;

	printf("\n%s Color table:\n============\n", (sd->glob_colors) ? "Global" : "Local");

	if (sd->glob_colors)
		len = sd->glob_colors_len_real;
	else
		len = id->local_colors_len_real;

	for (int i = 0; i < len; i++) {
		printf("|- color[%d]: #%x%x%x\n", i, img->colors_tab[i].r,
				img->colors_tab[i].g, img->colors_tab[i].b);
	}
}

static void gif_print_img_desc(struct image_descriptor *id)
{
	printf("\nImage Descriptor:\n=================\n");
	printf("|- left position: %d\n", id->image_left);
	printf("|- top position: %d\n", id->image_top);
	printf("|- width: %d\n", id->image_width);
	printf("|- height: %d\n", id->image_height);
	printf("|- local color tab. flag: %s\n", GET_T_OR_F(id->local_color_table_flag));
	if (id->local_color_table_flag)
	{
		printf("|- local color tab. size: %d\n", id->local_colors_len);
		printf("|- local color tab. size (real): %d\n", id->local_colors_len_real);
	}
	printf("|- interlance flag: %s\n", GET_T_OR_F(id->interlance));
	printf("|- sort flag: %s\n", GET_T_OR_F(id->sort));
}

static void gif_print_lzw_data(tGIF *img)
{
	printf("\nImage Data\n==========\n");
	printf("|- LZW minimum code size: %d\n", img->lzw_min_code_size);

	struct list_head *pos;
	struct sub_block *block;
	int block_count = 0;
	list_for_each(pos, &(img->data_lzw.list)) {
		block = list_entry(pos, struct sub_block, list);
		printf("|- block(%d) size: %d\n", block_count, block->size);

		for (int i = 0; i < block->size; i++) {
			printf("   |- data[%d]: bin=", i);
			byte_bin_str(block->data[i]);
			printf(", hex=0x%X, dec=%d\n", block->data[i], block->data[i]);
		}
		block_count++;
  }
}

int gif_print_info(FILE *in)
{
	tGIF *img;

	// load image into memory
	if ((img = gif_load(in)) == NULL)
		return 1;

	gif_print_file_info(img);
	gif_print_header(img);
	gif_print_lsd(&(img->screen_desc));
	gif_print_color_table(img, true);
	gif_print_img_desc(&(img->image_desc));
	gif_print_color_table(img, false);
	gif_print_lzw_data(img);

	gif_unload(img);
	return 0;
}
