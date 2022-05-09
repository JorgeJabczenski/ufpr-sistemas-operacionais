# GRR20190372 Jorge Lucas Vicilli Jabczenski

CC = gcc
CFLAGS = -Wall -g 
LDLIBS = -lm
 
objs = queue.o ppos_core.o
 
all: pingpong-mqueue

pingpong-mqueue: $(objs)

clean:
	-rm -f $(objs) *~
 
purge: clean
	-rm -f pingpong-mqueue