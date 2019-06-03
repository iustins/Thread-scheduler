CC=gcc
CFLAGS=-Wall -g -fPIC
LDFLAGS=
build: libscheduler.so

libscheduler.so: list.o priority_queue.o scheduler.o so_scheduler.o
	${CC} ${LDFLAGS} -shared $^ -o $@ ${CFLAGS}

clean:
	rm -f *.o libscheduler.so
