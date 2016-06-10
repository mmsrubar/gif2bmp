#ifndef GIF_DICT_H
#define GIF_DICT_H

#include "gif.h"

/* 
 * The maximum LZW code size can be 12 bits so the dictionary don't have to have
 * more then 2^12 entries.
 */
#define DICT_MAX_SIZE			4096
#define DICT_SENTINEL			','
#define DICT_SENTINEL_STR	","

/*
 * n is index to the color table. Where there is longer string then one
 * character then the string represent more indexes separated with comma. For
 * example string "1,2,2,1\0" represents indexes 1,2,2 and 1.
 *
 *  ======================
 * | index | item         |
 *  ======================
 * | 0     | "n\0"        | 
 * | 1     | "n\0"        |
 * | ...   | ...          |
 * | 12    | "n1,n2,n3\0" |
 * | ...   | ...          |
 * | 89    | NULL					|  <-- first free item
 * | ...   | ...          |
 *  ----------------------
 *
 *  There can be a gap between indexes from the color table and CC and EOI so
 *  always work with the free_item index as the last index of useful data!
 */
typedef struct dictionary {
	char **table;
	int free_item;								// index of the next free item
	int cc;												// index of the Clear Code
	int eoi;											// index of the End of Information code
} tGIF_DICT;

/*
 * Initialize the dictionary for use in the LZW decomopression. Don't forget to
 * call dict_free() before you call this function second time.
 *
 * @param img	GIF image
 * @param dict Dictionary
 *
 * @return 0 success
 * @return 1 error while allocating memory
 */
int dict_init(tGIF *img, tGIF_DICT *dict);

/*
 * Is code in the index table? Code is the index to the table. If there is not
 * NULL then the item exists.
 *
 * @param code will be used as an index to the table
 * @return true code is in the table
 * @return false code is not in the table
 */
bool dict_search(tGIF_DICT *dict, int code);

/*
 * Add a new {code}+k to the dictionary.
 *
 * @param dict The dictionary
 * @param code previous code
 * @param the K
 * @return index that the code was added to
 */
int dict_add(tGIF_DICT *dict, int code, const char *k);

/*
 * Get first index of item on position code. If the item contains string
 * "2,3,3,4\0" then the first index is number 2. The index is returned as a
 * string. The returned string has to be passed to free() after use!
 *
 * @param dict The dictionary
 * @param code previous code
 */
char *dict_get_k(tGIF_DICT *dict, int code);

#ifdef DEBUG
void dict_show(tGIF_DICT *dict);
#endif

/*
 * Free the dictionary from the memory.
 *
 * @param dict The dictionary
 */
void dict_free(tGIF_DICT *dict);

#endif	// GIF_DICT_H
