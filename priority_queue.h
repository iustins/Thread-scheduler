#ifndef _PRIORITY_QUEUE_H_
#define _PRIORITY_QUEUE_H_
#include "so_scheduler.h" /* SO_MAX_PRIO */
#include "list.h"

struct so_thread;

typedef struct {
	list *lists[SO_MAX_PRIO + 1];
} priority_queue;

priority_queue *alloc_priority_queue(void);
void delete_priority_queue(priority_queue *pq);
int add_to_priority_queue(priority_queue *pq,
		unsigned int priority, struct so_thread *thread);
struct so_thread *get_from_priority_queue(priority_queue *pq);
int highest_present_priority(priority_queue *pq);
int remove_from_priority_queue(priority_queue *pq);
int is_priority_queue_empty(priority_queue *pq);

#endif /* _PRIORITY_QUEUE_H_ */
