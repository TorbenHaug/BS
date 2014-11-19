CC=/usr/bin/gcc
CCFLAGS := -Wall -std=c99 -pthread -D_XOPEN_SOURCE=600
LDFLAGS := -lpthread
CCFLAGS += -DDEBUG_MESSAGES
CCFLAGS += -DVMEM_ALGO=2 #0=FIFO, 1=LRU, 2=Clock

all: vmappl mmanage

# VMAPPL COMPILIEREN

vmappl.o: vmappl.c vmappl.h
		$(CC) $(CCFLAGS) -c -o vmappl.o vmappl.c
vmaccess.o: vmaccess.c vmaccess.h
		$(CC) $(CCFLAGS) -c -o vmaccess.o vmaccess.c
vmappl: vmaccess.o vmappl.o
		$(CC) $(CCFLAGS) -o vmappl vmaccess.o vmappl.o

# MMANAGE COMPILIEREN

mmanage.o: mmanage.c mmanage.h
		$(CC) $(CCFLAGS) -c -o mmanage.o mmanage.c

mmanage: mmanage.o
		$(CC) $(CCFLAGS) -o mmanage mmanage.o
		
clean: 
	rm *.o mmanage vmappl logfile.txt pagefile.bin
