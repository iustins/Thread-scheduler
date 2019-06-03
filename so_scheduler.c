#include "so_scheduler.h"
#include "scheduler.h"

static struct scheduler *sched;

int so_init(unsigned int time_quantum, unsigned int io)
{
	if (sched)
		return -1;
	sched = create_scheduler(time_quantum, io);
	if (!sched)
		return -1;
	return 0;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	return create_new_thread(sched, func, priority);
}

int so_wait(unsigned int io)
{
	return wait_on_device(sched, io);
}

int so_signal(unsigned int io)
{
	return signal_device(sched, io);
}

void so_exec(void)
{
	scheduler_do_exec(sched);
}

void so_end(void)
{
	if (sched) {
		wait_for_scheduler_to_end(sched);
		sched = NULL;
	}
}
