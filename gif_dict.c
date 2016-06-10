/*
 * Autor: Michal Šrubař
 * Email: xsruba03@stud.fit.vutbr.cz
 * Date: 	Sat Mar  5 15:26:53 CET 2016
 * Modul: Dictionary
 *
 * This module impplement functions for GIF dictionary which is used during the
 * LZW decompression.
 *
 */

#define _XOPEN_SOURCE  700

#include "gif_dict.h"
#include "gif_bits.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* Convert 4 digit number in range of 0 to 9999 to a string representation */
static char *int_to_str(int i)
{
	char *str;

	if (i > 9999)
		handle_error("Can conver only 1 byte long numbers", NULL);

	if ((str = (char *) malloc(sizeof(char) * 5)) == NULL)	// three digits + \0
		handle_error("Error while allocating memory", NULL);

	sprintf(str, "%d", i);

	return str;
}

/* Concatenate string str1 with string str2 and put results back to the str1 */
static void dict_string_cat(char **str1, const char *str2)
{
	/* nothing needs to be done if there is no string in str2 */
	if (str2 == NULL || strlen(str2) == 0)
		return;
	if (*str1 == NULL || strlen(*str1) == 0) {
		*str1 = strdup(str2);
		return;
	}

	/* there is at least one character in both str1 and str2 strings */
	int len = strlen(*str1) + strlen(str2) + 2;	// +2 for ',' and '\0'
	char *new = malloc(sizeof(char) * len);
	strcpy(new, *str1);
	strcat(new, DICT_SENTINEL_STR);
	strcat(new, str2);
	free(*str1);
	*str1 = new;
}
	
// return index of Clear Code (CC)
int dict_init(tGIF *img, tGIF_DICT *dict)
{
	dict->free_item = -1;
	dict->cc = -1;
	dict->eoi = -1;

	if ((dict->table = malloc(sizeof(char *) * DICT_MAX_SIZE)) == NULL)
		handle_error("Error while allocating memory", 1);

	// clear out the entire table first
	memset(dict->table, 0, sizeof(char *) * DICT_MAX_SIZE);

	// add all color indexes from the color table to the dictionary
	int colors_count = (img->screen_desc.glob_colors) ? 
		img->screen_desc.glob_colors_len_real : 
		img->image_desc.local_colors_len_real;

	for (int i = 0; i < colors_count; i++)
	{
		if ((dict->table[i] = int_to_str(i)) == NULL)
			handle_error("Error while allocating memory", 1);

		dict->free_item = i + 1;
	}

	// add Clear (CC) and End of Informatinn (EOI) codes
	int cc_index = POW2(img->lzw_min_code_size);

	if ((dict->table[cc_index] = strdup("CC")) == NULL)
		handle_error("Error while allocating memory", 1);
	if ((dict->table[cc_index+1] = strdup("EOI")) == NULL)
		handle_error("Error while allocating memory", 1);

	dict->cc = cc_index;
	dict->eoi = cc_index+1;
	dict->free_item = cc_index+2;

	return 0;
}

bool dict_search(tGIF_DICT *dict, int code)
{
	if (dict->table[code] != NULL)
		return true;
	else
		return false;
}

char *dict_get_k(tGIF_DICT *dict, int code)
{
	if (dict->table[code] == NULL)
		return NULL;

	if (strchr(dict->table[code], DICT_SENTINEL) == NULL)
	{
		// e.g. "12\0" - just one index
		return strdup(dict->table[code]);
	}

	// "1,3,3,..\0" - more indexes
	char *copy = strndup(dict->table[code], strlen(dict->table[code]));
	char *tmp = strdup(strtok(copy, DICT_SENTINEL_STR));
	free(copy);
	return tmp;
}

int dict_add(tGIF_DICT *dict, int code, const char *k)
{
	char *copy = NULL;

	/* we don't want change the item in the dictionary */
	if (dict->table[code] != NULL)
		copy = strdup(dict->table[code]);
	/* concatenate copy and k strings (we add the sentinel too)*/
	dict_string_cat(&copy, k);
	/* add the new item to the dictionary */
	dict->table[dict->free_item] = copy;
	/* return index we put item on and then incremet */
	return dict->free_item++;
}

#ifdef DEBUG
void dict_show(tGIF_DICT *dict)
{
	printf("Dictionary:\n");
	for (int i = 0; i < dict->free_item; i++)
		printf("dict[%d]: %s\n", i, dict->table[i]);
	printf("next free index: %d\n", dict->free_item);
	printf("===================\n");
}
#endif

void dict_free(tGIF_DICT *dict)
{
	/* Use free_item index as a sentinel because there can be a NULL gap between
	 * color indexes and Clear code.
	 */
	for (int i = 0; i < dict->free_item; i++)
		free(dict->table[i]);

	free(dict->table);
	dict->table = NULL;
}
