CC = gcc
CFLAGS = -Wall -g
 
objs = queue.o ppos_core.o
 
all: semaphore racecond 

semaphore: $(objs)

racecond: $(objs)

clean:
	-rm -f $(objs) *~
 
purge: clean
	-rm -f semaphore racecond