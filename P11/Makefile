CC = gcc
CFLAGS = -Wall -g 
 
objs = queue.o ppos_core.o
 
all: pingpong-prodcons 

pingpong-prodcons : $(objs)

clean:
	-rm -f $(objs) *~
 
purge: clean
	-rm -f pingpong-prodcons 