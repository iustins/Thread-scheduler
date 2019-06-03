#include <stdlib.h>
#include "scheduler.h"

struct scheduler *create_scheduler(unsigned int time_quantum, unsigned int io)
{
	struct scheduler *result = calloc(sizeof(*result), 1);

	if (!result)
		return NULL;
	if (time_quantum == 0 || io > SO_MAX_NUM_EVENTS)
		goto cleanup;
	result->ready = alloc_priority_queue();
	if (!result->ready)
		goto cleanup;
	result->devices = calloc(sizeof(list *), io);
	if (!result->devices)
		goto cleanup;
	for (unsigned int i = 0; i < io; i++) {
		result->devices[i] = alloc_list();
		if (!result->devices[i])
			goto cleanup;
	}
	result->active_thread = NULL;
	result->master_thread = create_master_thread();
	if (!result->master_thread)
		goto cleanup;
	result->device_count = io;
	result->thread_to_join = INVALID_TID;
	result->time_quantum = time_quantum;
	result->time_spent = 0;
	result->thread_count = 0;
	return result;
cleanup:
	if (!result)
		return NULL;
	if (result->ready)
		delete_priority_queue(result->ready);
	if (result->devices) {
		for (unsigned int i = 0; i < io; i++)
			if (result->devices[i])
				delete_list(result->devices[i]);
		free(result->devices);
	}
	free(result);
	return NULL;
}

struct thread_startup_info {
	struct scheduler *sched;
	so_handler *handler;
	unsigned int priority;
};

static void *scheduler_thread_startup(void *arg)
{
	struct thread_startup_info *info = (struct thread_startup_info *)arg;
	/* TODO */
	struct so_thread *thread = create_thread_structures();
	int rc;

	if (!thread) {
		free(info);
		return NULL;
	}
	thread->priority = info->priority;
	/* If I've been created by the master thread
	 * I can go ahead and do work
	 */
	if (info->sched->active_thread) {
		rc = add_to_priority_queue(info->sched->ready,
				info->priority, thread);
		if (rc < 0) {
			free(thread);
			free(info);
			return NULL;
		}
		/* After adding myself to the ready queue, I can be scheduled */
		/* Another thread is running */
		/* Wake it up and put myself to sleep */
		wake_thread(info->sched->active_thread);
		block_thread(thread);
	}
	/* Either I'm the first thread or I've been scheduled */
	info->sched->active_thread = thread;
	info->sched->time_spent = 0;
	if (!pthread_equal(info->sched->thread_to_join, INVALID_TID)) {
		pthread_join(info->sched->thread_to_join, NULL);
		info->sched->thread_to_join = INVALID_TID;
	}
	info->handler(info->priority);
	/* Thread about to end */
	/* Ensure the "preempt" function won't block us */
	wake_thread(info->sched->active_thread);
	/* We have to set up the thread that is about to die */
	info->sched->thread_to_join = pthread_self();
	context_switch(info->sched);
	delete_thread(thread);
	/* And it's done */
	return NULL;
}

pthread_t create_new_thread(struct scheduler *sched, so_handler *handler,
		unsigned int priority)
{
	struct thread_startup_info *info = malloc(sizeof(*info));
	pthread_t tid;
	struct so_thread *active_thread = get_active_thread(sched);

	if (!info)
		return INVALID_TID;
	if (!handler) {
		free(info);
		return INVALID_TID;
	}
	if (priority > SO_MAX_PRIO) {
		free(info);
		return INVALID_TID;
	}
	info->sched = sched;
	info->handler = handler;
	info->priority = priority;
	if (pthread_create(&tid, NULL, scheduler_thread_startup, info) < 0)
		return INVALID_TID;
	sched->thread_count = 1;
	if (active_thread != NULL) {
		/* We shall wait for the child to wake us up */
		block_thread(active_thread);
		/* Add to the time quantum, preempt etc */
		scheduler_do_exec(sched);
	}
	return tid;
}

int wait_on_device(struct scheduler *sched, unsigned int io)
{
	return -1;
}

int signal_device(struct scheduler *sched, unsigned int io)
{
	return -1;
}

void scheduler_do_exec(struct scheduler *sched)
{
	struct so_thread *active_thread = get_active_thread(sched);

	sched->time_spent++;
	if (sched->time_spent >= sched->time_quantum) {
		preempt(sched);
		return;
	}
	if (highest_present_priority(sched->ready) > active_thread->priority)
		preempt(sched);
}

struct so_thread *get_active_thread(struct scheduler *sched)
{
	return sched->active_thread;
}

void wait_for_scheduler_to_end(struct scheduler *scheduler)
{
	if (scheduler->thread_count == 0)
		wake_thread(scheduler->master_thread);
	block_thread(scheduler->master_thread);
	/* Master thread is awake */
	pthread_join(scheduler->thread_to_join, NULL);
	delete_priority_queue(scheduler->ready);
	for (int i = 0; i < scheduler->device_count; i++)
		delete_list(scheduler->devices[i]);
	delete_thread(scheduler->master_thread);
	free(scheduler);
}

struct so_thread *create_master_thread(void)
{
	struct so_thread *result = malloc(sizeof(*result));

	if (sem_init(&result->semaphore, 0, 0) < 0) {
		/* This won't happen */
		free(result);
		return NULL;
	}
	return result;
}

struct so_thread *create_thread_structures(void)
{
	return create_master_thread(); /* Identical implementation */
}

void delete_thread(struct so_thread *thread)
{
	sem_destroy(&thread->semaphore);
	free(thread);
}

void wake_thread(struct so_thread *thread)
{
	sem_post(&thread->semaphore);
}

void block_thread(struct so_thread *thread)
{
	sem_wait(&thread->semaphore);
}

void context_switch(struct scheduler *scheduler)
{
	struct so_thread *next_thread =
		get_from_priority_queue(scheduler->ready);
	struct so_thread *current_thread = scheduler->active_thread;

	remove_from_priority_queue(scheduler->ready);
	if (!next_thread)
		wake_thread(scheduler->master_thread);
	else
		wake_thread(next_thread);
	block_thread(current_thread);
	/* We are now the active thread */
	scheduler->active_thread = current_thread;
	if (pthread_equal(scheduler->thread_to_join, pthread_self()))
		return; /* Skip attempting to join */
	if (!pthread_equal(scheduler->thread_to_join, INVALID_TID)) {
		pthread_join(scheduler->thread_to_join, NULL);
		scheduler->thread_to_join = INVALID_TID;
	}
}

void preempt(struct scheduler *scheduler)
{
	add_to_priority_queue(scheduler->ready,
			scheduler->active_thread->priority,
			scheduler->active_thread);
	context_switch(scheduler);
}
