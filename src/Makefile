CC=gcc
CFLAGS=-Wall -g
OBJS=server.o resp_gen.o

all: server	

server: $(OBJS)
	gcc -o $@ $^

resp_gen.o: resp_gen.c resp_gen.h

clean:
	rm -f $(OBJS)
	rm -f server
	rm -f resp_gen

