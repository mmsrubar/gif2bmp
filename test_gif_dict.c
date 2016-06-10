#define _XOPEN_SOURCE 700

#include "gif_dict.h"
#include <string.h>
#include <stdlib.h>

#define test_add_indexes(i, s) \
	indexes = i; \
	str = s; \
	printf("dict_string_cat(%s, %s)", indexes, str); \
	dict_string_cat(&indexes, str); \
	printf(" \t--> %s\n", indexes);

int main(int argc, char *argv[])
{
	char *indexes = NULL;
	char *str = NULL;

	printf("TEST: fucntion dict_string_cat()\n================================\n");
	test_add_indexes(NULL, NULL);
	test_add_indexes(NULL, "");
	test_add_indexes(NULL, "1");
	test_add_indexes(NULL, "1,2");
	test_add_indexes(strdup(""), NULL);
	test_add_indexes(strdup(""), "");
	test_add_indexes(strdup("1"), NULL);
	test_add_indexes(strdup("1,2"), NULL);
	test_add_indexes(strdup(""), "");
	test_add_indexes(strdup(""), "0");
	test_add_indexes(strdup(""), "0,1");

	test_add_indexes(strdup("1"), "");
	test_add_indexes(strdup("1"), "1");
	test_add_indexes(strdup("1"), "1,3");

	test_add_indexes(strdup("1,2"), "");
	test_add_indexes(strdup("1,2"), "2");
	test_add_indexes(strdup("1,2"), "3,4");

	free(indexes);

	return 0;
}
