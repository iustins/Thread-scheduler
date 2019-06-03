#ifndef _LIST_H_
#define _LIST_H_

struct so_thread;

typedef struct list_node {
	struct so_thread *thread;
	struct list_node *next;
} list_node;

typedef struct {
	list_node *first;
	list_node *last;
} list;

list *alloc_list(void);
void delete_list(list *l);
int add_to_list(list *l, struct so_thread *thread);
struct so_thread *get_from_list(list const *l);
int remove_from_list(list *l);
int is_list_empty(list *l);

#endif /* _LIST_H_ */
