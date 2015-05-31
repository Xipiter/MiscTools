CC = gcc
CFLAGS = -g #-O -Wall -Werror
TARGET = s1cflatline makes1cfifo

all: $(TARGET)

s1cflatline: s1cflatline.o
	$(CC) -o s1cflatline s1cflatline.o 

makes1cfifo: makes1cfifo.o
	$(CC) -o makes1cfifo makes1cfifo.o 

.c.o:
	$(CC) $(CFLAGS) -c $<

clean:
	rm -rf makes1cfifo s1cflatline .s1cflatlinefifo *.o *~ core *.core logs files

install:
	./makes1cfifo
	mkdir logs
	mkdir files
	@echo "A 'make clean' will wipe your ./logs and ./files, be sure to back them up."

