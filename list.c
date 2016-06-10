#include <stdio.h>
#include <stdlib.h>
#include "list.h"

struct list_item {
  int val;
	struct list_head list;
};

int main(int argc, char **argv)
{
	struct list_item *new, *item, *tmp;
	struct list_head *pos, *q;
	unsigned int i;

	struct list_item mylist;
	INIT_LIST_HEAD(&mylist.list);

  new = (struct list_item *)malloc(sizeof(struct list_item));
  new->val = 10; 
  list_add_tail(&(new->list), &(mylist.list));

  printf("Iterate with list_for_each_entry()\n");
  list_for_each_entry(item, &(mylist.list), list) {
    printf("val: %d\n", item->val);
  }
  putchar('\n');

	/* now let's be good and free the list_item items. since we will be removing items
	 * off the list using list_del() we need to use a safer version of the list_for_each() 
	 * macro aptly named list_for_each_safe(). Note that you MUST use this macro if the loop 
	 * involves deletions of items (or moving items from one list to another). */
	printf("deleting the list using list_for_each_safe()\n");
	list_for_each_safe(pos, q, &mylist.list){
    item = list_entry(pos, struct list_item, list);
    list_del(pos);  /* 1st remove item from the list */
    free(item);     /* 2nd free the entire item */
	}

  printf("Iterate with list_for_each_entry()\n");
  list_for_each_entry(item, &(mylist.list), list) {
    printf("val: %d\n", item->val);
  }

  printf("Is the list empty? %s\n", list_empty(&mylist.list) ? "yes" : "no");

	return 0;
}
