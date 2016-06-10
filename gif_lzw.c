#define _XOPEN_SOURCE 700

/*
 *
 * Autor: Michal Šrubař
 * Email: xsruba03@stud.fit.vutbr.cz
 * Date: 	Sat Mar  5 15:26:53 CET 2016
 * Modul: GIF LZW Decompression
 *
 * This module implement the LZW Decompression Algorithm.
 *
 */

#include "gif_lzw.h"
#include "gif_dict.h"
#include "gif_bits.h"
#include "gif_interlance.h"
#include <stdlib.h>
#include <string.h>

#define COMPRESSION_SIZE_MAX	12

/* The compression size during the LZW decompression is dynamic. When the size
 * is 3b and we write into the table on index 7 (2^3-1) then we have to increase
 * the compression size.
 */
#define SIZE_TEST(i, s) { if (i == POW2(s) - 1) s++; }
/* Initialize the compression size = how much bits a fraze consits of */
#define COMPRESSION_SIZE_INIT(c, i) (c = i->lzw_min_code_size + 1)

/* current block the algorighm is working with */
struct list_head *head;
static struct list_head *block;	// pointer to current block in list of blocks
static int block_size;					// number of blocks bytes
static u_int8_t *data;					// blocks bytes
static int compression_size;		// with how many bits are we working with

/*
 * Initialize the compression size and the dictionary which will be used for
 * phrases.
 *
 * @return
 * 0 Initialization was successful
 * 1 The first block in not a Clear Code (CC)
 */
static void lzw_init(tGIF *img)
{
	struct sub_block *sub_block;

	COMPRESSION_SIZE_INIT(compression_size, img);
	head = &(img->data_lzw.list);
	block = head->next;	// get the first block of data
	sub_block = list_entry(block, struct sub_block, list);
	data = sub_block->data;
	block_size = sub_block->size;

	// initialize a structure for uncompressed data
	img->data.indexes = NULL;
	img->data.size = 0;
}

/* 
 * Handles situations when I need to read code which is spread in two bytes.
 * For example I have the following bytes:
 * 						765543210
 * 		byte1:	1100 0000
 * 		byte2:	0000 0001
 * and my bit_idx points to bit 6 and I need to read code of size 3. Then the
 * code will be 111 (1st 1 from byte2 and last two 1's from byte1).
 */
static void safe_byte_idx_increment(int *byte_idx, int *bit_idx)
{
	struct sub_block *sub_block;

	/* If we get behind the range of the byte then we have to move this this bit
	 * pointer to another byte which can be:
	 * 	1. in the same subblock or
	 * 	2. in next subblock.
	 */
	if (*bit_idx+1 == 8)
	{
		if (*byte_idx+1 >= block_size)
		{
			// 2. next byte is in the next subblock
			block = block->next;
			sub_block = list_entry(block, struct sub_block, list);
			data = sub_block->data;
			block_size = sub_block->size;
			*byte_idx = 0;
			(*bit_idx) = 0;
		}
		else
		{
	 		// 1. byte is in the same subblock
			(*byte_idx)++;
			////printf("--> switch to byte: %d\n", *byte_idx);
			(*bit_idx) = 0;
		}
	}
	else
		(*bit_idx)++;
}

/*
 * Return another index from the compressed byte stream. This index is the index
 * to the color table. First returned index is the index of the color of the
 * top-left pixel in the GIF image. Every other call returns idexex of following
 * pixels colors.
 *
 * Example:
 *
 * List of subblocks (represents all compressed data)
 *  --------       --------               --------
 * | block1 | --> | block2 | --> ... --> | blockN |
 *  --------       --------               --------
 *    |
 *    |
 *  -----------------
 * | subblock:       |
 * | size = 4				 |
 * | data =  (array) |
 * | 0 1100101       |
 * | 1 00:101:100    |
 * | 2 01010100      |
 * | 3 10100111      |
 *  -----------------
 * Let's say that we have the following byte stream from data array in the
 * sublock and we are working with the 1st byte from left which is 00101100.
 * Then the byte_idx would be se to 1. Now suppose the compression size is 3 and
 * we working with the 5th-4th bits of the byte which are 101. Then the bit_idx
 * will be set to 4. Now the value 101 is the index to the table of indexes
 * (dictionary).
 *
 * @param size 	Compression size
 * @param init	True on very first call and then false all other calls
 */
static int get_code(bool init)
{
	static int byte_idx = 0;
	static int bit_idx = 0;		/* points to the bit I'll start read data from */
	int code = 0;
	int n = 0;

	if (init) {
		byte_idx = 0;
		bit_idx = 0;
	}

	if (compression_size == COMPRESSION_SIZE_MAX+1)
	{
		// read only 12 bits because this code should be CC 
		compression_size = 12;
	}

	for (int i = 0; i < compression_size; i++)
	{
		if (GET_BIT(data[byte_idx], bit_idx) != 0)
			code += POW2(n);

		n++;
		safe_byte_idx_increment(&byte_idx, &bit_idx);
	}

	return code;
}

#ifdef DEBUG
static void pixels_count_test(tGIF *img)
{
	printf("indexes:%d == %ld pixels (widht*height)\n", img->data.size, 
			img->image_desc.image_width*img->image_desc.image_height);
}
#endif

/*
 * Save a single index (represented as a string) to the array of indexes to the
 * color table.
 */
static int gif_lzw_save_token(tGIF *img, char *str)
{
	if (str == NULL)
		handle_error("error: str==NULL", 1);

	img->data.indexes = realloc(img->data.indexes, sizeof(u_int8_t) * (img->data.size + 1));
	if (img->data.indexes == NULL)
		handle_error("malloc() error", 1);

	// add new index (size represents also pointer to newly allocated byte */
	img->data.indexes[img->data.size] = (u_int8_t) atoi(str);
	img->data.size += 1;
	return 0;
}

/*
 * Save uncompressed index(s) to the array of uncompressed indexes. The code
 * parameter is used as an index to the dictionary where there is a string
 * representing one or more indexes, for example: "1,2,2" or just "5".
 *
 * @param code an index to the dictionary
 */
static int gif_lzw_save_indexes(tGIF *img, tGIF_DICT *dict, int code)
{
	/* first copy the string from the dictionary */
	char *str = strdup(dict->table[code]);

	if (str == NULL)
	{
		handle_error("No indexes in the dictionary for this code", 1);
	}
	else if (strchr(str, DICT_SENTINEL) == NULL)
	{
		// there is only one index in the string
		if (gif_lzw_save_token(img, str) != 0)
			return 1;
	}
	else
	{
		// there is more then one index in the string
		char *token = strtok(str, DICT_SENTINEL_STR);

		do {
			if (gif_lzw_save_token(img, token) != 0)
				return 1;
		} while ((token = strtok(NULL, DICT_SENTINEL_STR)) != NULL);
	}

	free(str);
	return 0;
}

int gif_lzw_decompression(tGIF *img)
{
	tGIF_DICT dict;
	int code;
	int code_prev;
	char *k;
	int idx;

	lzw_init(img);

	/* The LZW Decompression algorithm:
	 * ================================
	 */

	/* initializaze the dictionary first */
	if (dict_init(img, &dict) == 1)
		handle_error("Can't initialize the dictionary", 1);
	
	/* first code should be the Clear Code */
	if (get_code(true) != dict.cc)	
		handle_error("Something went wrong. The first code isn't the Clear Code", 1);

	/* read first code and put it to the array of color table indexes */ 
	code = get_code(false);

	/* save uncompressed index */
	if (gif_lzw_save_indexes(img, &dict, code) == 1)
		handle_error("Can't save index to uncompressed data array", 1);


	/* MAIN LOOP (until we read Clear Code or End of Information code) */
	while (true)
	{
		/* read another code */
		code_prev = code;
		code = get_code(false);

		if (code == dict.eoi)
			break;
		if (code == dict.cc) {
			dict_free(&dict);
			dict_init(img, &dict);
			COMPRESSION_SIZE_INIT(compression_size, img);
			/* read first code and put it to the string of color table indexes*/ 
			code = get_code(false);
			if (gif_lzw_save_indexes(img, &dict, code) == 1)
				handle_error("Can't save index to uncompressed data array", 1);
			continue;
		}

		/* is code in the table yet? */
		if (dict_search(&dict, code) == true)
		{
			if (gif_lzw_save_indexes(img, &dict, code) == 1)
				handle_error("Can't save index to uncompressed data array", 1);
			/* get first index from {code} */
			k = dict_get_k(&dict, code);
		}
		else
		{
			/* get first index from {code-1} */
			k = dict_get_k(&dict, code_prev);

			if (gif_lzw_save_indexes(img, &dict, code_prev) == 1)
				handle_error("Can't save index to uncompressed data array", 1);
			if (gif_lzw_save_token(img, k) == 1)
				handle_error("Can't save index to uncompressed data array", 1);
		}

		/* add {code-1}+K into the dictionary */
		idx = dict_add(&dict, code_prev, k);
		free(k);
		SIZE_TEST(idx, compression_size);
	} 

	// if interlance is on then we have to switch some lines :)
	if (img->image_desc.interlance)
		gif_lzw_interlacing(img);

	dict_free(&dict);

#ifdef DEBUG
	pixels_count_test(img);

	printf("Uncompressed data:\n");
	if (img->data.size != img->image_desc.image_width*img->image_desc.image_height)
	{
		printf("nesedi velikost dekomprimovanych dat a sirka*vyska\n");
		return 1;
	}

	int row = 0;
	int w = 0;
	printf("row[%d]: ", row);
	for (int i = 0; i < img->data.size; i++)
	{
		printf("%d,", img->data.indexes[i]);

		w++;
		if (w == img->image_desc.image_width)
		{
			printf("\n");
			printf("row[%d]: ", ++row);
			w = 0;
		}
	}
#endif

	return 0;
}
