#include <stdlib.h>
#include "list.h"

list *alloc_list(void)
{
	list *result = malloc(sizeof(*result));

	if (!result)
		return NULL;
	result->first = NULL;
	result->last = NULL;
	return result;
}

void delete_list(list *l)
{
	while (!is_list_empty(l))
		remove_from_list(l);
	free(l);
}

int add_to_list(list *l, struct so_thread *thread)
{
	list_node *node = malloc(sizeof(*node));

	if (!node)
		return -1;
	node->thread = thread;
	node->next = NULL;
	if (l->last == NULL) {
		l->first = node;
		l->last = node;
		return 0;
	}
	l->last->next = node;
	l->last = node;
	return 0;
}

struct so_thread *get_from_list(list const *l)
{
	if (!l || !l->first)
		return NULL;
	return l->first->thread;
}

int remove_from_list(list *l)
{
	list_node *tmp;

	if (!l || !l->first)
		return -1;
	tmp = l->first;
	l->first = l->first->next;
	if (!l->first)
		l->last = NULL;
	free(tmp);
	return 0;
}

int is_list_empty(list *l)
{
	if (!l)
		return -1;
	return !l->first;
}
