VERSION = 3.02
CC	= /usr/bin/gcc
CFLAGS	= -Wall -g -D_REENTRANT -DVERSION=\"$(VERSION)\"
LDFRALGS= -lm -lpthread `gtk-config --cflags` `gtk-config --libs` -lgthread

OBJ = consumer.o control.o getch.o main.o producer.o

compile: $(OBJ)
	$(CC) $(CFLAGS) -o compile $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<
