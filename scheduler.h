#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_
#include <semaphore.h>
#include <pthread.h>
#include "priority_queue.h"
#include "so_scheduler.h"

struct so_thread {
	sem_t semaphore;
	unsigned int priority;
};

struct scheduler {
	priority_queue *ready;

	list **devices;
	struct so_thread *active_thread;
	int device_count;
	pthread_t thread_to_join;
	unsigned int time_quantum;
	unsigned int time_spent;
	int thread_count;
	struct so_thread *master_thread;
};

struct scheduler *create_scheduler(unsigned int time_quantum, unsigned int io);
pthread_t create_new_thread(struct scheduler *scheduler, so_handler *handler,
		unsigned int priority);
int wait_on_device(struct scheduler *scheduler, unsigned int io); /* TODO */
int signal_device(struct scheduler *scheduler, unsigned int io); /* TODO */
void scheduler_do_exec(struct scheduler *scheduler);
struct so_thread *get_active_thread(struct scheduler *scheduler);
void wait_for_scheduler_to_end(struct scheduler *scheduler);

/* Thread management */

struct so_thread *create_master_thread(void);
struct so_thread *create_thread_structures(void);
void delete_thread(struct so_thread *thread);
void wake_thread(struct so_thread *thread);
void block_thread(struct so_thread *current_thread);
void context_switch(struct scheduler *scheduler);
void preempt(struct scheduler *scheduler);

#endif /* _SCHEDULER_H_ */
