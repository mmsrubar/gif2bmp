#ifndef GIF_H
#define GIF_H

#include "list.h"
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>

#define SIGNATURE							"GIF"
#define VERSION1 							"89a"
#define VERSION2 							"87a"
#define COLOR_TAB_LEN_MAX			256
#define SUBBLOCK_LEN_MAX			256

#define IMAGE_SEPARATOR 			0x2C
#define EXTENSION							0x21
#define GRAPHICS_CTRL_LABEL		0xF9
#define PLAIN_TEXT_LABEL			0x01
#define APP_EXTENSION_LABEL		0xFF
#define COMMENT_LABEL					0xFE
#define TRAILER 							0x3B

#define HEADER_SIZE							6
#define LOCAL_SCREEN_DESC_SIZE	7
#define COLOR_SIZE							3
#define GRAPHICS_CTRL_EXT_SIZE							8
#define APP_EXT_SIZE			19
#define PLAIN_TEXT_EXT_SIZE			28
#define IMAGE_DESC_SIZE 10

/* handle error return values */
#define handle_error(msg, ret) { fprintf(stderr, "%s\n", msg); return ret; }

struct logical_screen_descriptor {
	// Logical Screen Descriptor
	u_int16_t width;
	u_int16_t height;
	bool glob_colors;
	u_int8_t glob_colors_len;		// lenght of the global color table
	int glob_colors_len_real; // 2^(len + 1)
	u_int8_t color_resolution;	// bits/pixel
	bool sort;
	u_int8_t background_color;
	u_int8_t pixel_aspect_ratio;
};

struct color {
	u_int8_t r;	// red
	u_int8_t g;	// green
	u_int8_t b;	// blue
};

struct image_descriptor {
	u_int16_t image_left;
	u_int16_t image_top;
	u_int16_t image_width;
	u_int16_t image_height;
	bool local_color_table_flag;			// local color table flag
	bool interlance;
	bool sort;
	bool reversed1;
	bool reversed2;
	u_int8_t local_colors_len;		// lenght of the local color table
	int local_colors_len_real;	// 2^(len + 1)
};

struct sub_block {
	/* Sub-block of data is an array of max SUBBLOCK_LEN_MAX (256) bytes where the
	 * first byte determines the length of the array.
	 */
	int size;
	u_int8_t *data;
	struct list_head list;
};

struct color_indexes {
	/* Uncompressed data are represented like a byte array. An uncompressed item
	 * is an index to the color table which with 8-bit GIF images can only be
	 * value in range <0,255>.
	 */
	u_int8_t *indexes;
	int64_t size;
};


typedef struct {
	// total size of the GIF image
	int size;

	// header
	u_int8_t signature[4];
	u_int8_t version[4];

	struct logical_screen_descriptor screen_desc;
	// we only work with static GIF so there will be just ome image
	struct image_descriptor image_desc;	
	struct color colors_tab[COLOR_TAB_LEN_MAX];
	
	/* Data has to be in a dynamic list because I don't know how many subblocks are
	 * gonna be in the gif file until I go through them.
	 */
	u_int8_t lzw_min_code_size;
	struct sub_block data_lzw;	// LZW compressed data 
	struct color_indexes data;	// uncompressed data

	/* NOT IMPELEMENTED
	 * Graphics Color Extension
	 * Application Extension
	 * Plain Text Extension
	 * Comment Extension
	 */
	
} tGIF;

/**
 * Parse GIF image and load its infomation into the dynamic tGIF structure which
 * will be returned. User is responsible for calling gif_unload which will free
 * the entrie structure.
 *
 * @param f pointer to a GIF image file
 * @return GIF strucutre containg all internal information about the GIF image
 */
tGIF *gif_load(FILE *f);

/**
 * Free the entire tGIF structure from the memory.
 * @param img pointer to a tGIF structure
 */
void gif_unload(tGIF *img);

/**
 * Print all information from the GIF image.
 * @param img pointer to a tGIF structure
 */
void gif_print(tGIF *img);

#endif	// GIF_H
