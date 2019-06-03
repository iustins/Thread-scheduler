#include <stdlib.h>
#include "priority_queue.h"

priority_queue *alloc_priority_queue(void)
{
	priority_queue *result = malloc(sizeof(*result));

	if (!result)
		return NULL;

	for (unsigned int i = 0; i <= SO_MAX_PRIO; i++)
		result->lists[i] = alloc_list();
	for (unsigned int i = 0; i <= SO_MAX_PRIO; i++)
		if (!result->lists[i])
			goto cleanup_onerr;
	return result;
cleanup_onerr:
	for (unsigned int i = 0; i <= SO_MAX_PRIO; i++)
		if (result->lists[i])
			delete_list(result->lists[i]);
	free(result);
	return NULL;
}

void delete_priority_queue(priority_queue *pq)
{
	for (unsigned int i = 0; i <= SO_MAX_PRIO; i++)
		delete_list(pq->lists[i]);
	free(pq);
}

int add_to_priority_queue(priority_queue *pq,
		unsigned int priority, struct so_thread *thread)
{
	if (!pq || priority < 0 || priority > SO_MAX_PRIO)
		return -1;
	return add_to_list(pq->lists[priority], thread);
}

struct so_thread *get_from_priority_queue(priority_queue *pq)
{
	int prio = highest_present_priority(pq);

	if (prio < 0)
		return NULL;
	return get_from_list(pq->lists[prio]);
}

int highest_present_priority(priority_queue *pq)
{
	if (!pq)
		return -1;
	for (int prio = SO_MAX_PRIO; prio >= 0; prio--)
		if (!is_list_empty(pq->lists[prio]))
			return prio;
	return -1;
}

int remove_from_priority_queue(priority_queue *pq)
{
	int prio = highest_present_priority(pq);

	if (prio < 0)
		return -1;
	return remove_from_list(pq->lists[prio]);
}

int is_priority_queue_empty(priority_queue *pq)
{
	int prio = highest_present_priority(pq);

	if (!pq)
		return -1;
	return prio < 0;
}
