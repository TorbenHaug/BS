/* File: vmem.h
 * Global Definitions for BSP3 sample solution
 * Model of virtual memory management
 */

#ifndef VMEM_H
#define VMEM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHMKEY          "./vmem.h" //TODO: muss auf eine Reale Datei zeigen
#define SHMPROCID       'C'

typedef unsigned int Bmword;    /* Frame bitmap */

/* Sizes */
#define VMEM_VIRTMEMSIZE 1024   /* Process address space / items */
#define VMEM_PHYSMEMSIZE  256   /* Physical memory / items */
#define VMEM_PAGESIZE       8   /* Items per page */
#define VMEM_NPAGES     (VMEM_VIRTMEMSIZE / VMEM_PAGESIZE)     /* Total number of pages */
#define VMEM_NFRAMES 	(VMEM_PHYSMEMSIZE / VMEM_PAGESIZE)   /* Number of available frames */
#define VMEM_BITS_PER_BMWORD     (sizeof(Bmword) * 8)

// Um anzuzeigen, welche Frames frei (Bit = 0) oder belegt (Bit = 1) sind, wird in einer Bitmap eine Bitmaske
// verwendet. Die Bitmaske wird mit einem unsigned int (~0U) initialisiert.
// Um "freie Plätze einzurichten", wird um die nötige Anzahl Frames innerhalb der Bitmaske links-geshiftet,
// um Nullen hinzuschieben. Die nötige Anzahl an Shiftvorgängen wird berechnet durch
// VMEM_FRAMES % VMEM_BITS_PER_BMWORD    ("Gesamtanzahl Frames" modulo "Bits pro BMWort == unsigned int == 32")
// Problem: Legt man fest, dass 32 Frames verfügbar sein sollen (was der Anzahl Bits entspricht),
// ergibt die Modulo-Rechnung == 0.
// Damit würde in der Bitmaske überhaupt nicht geshiftet werden, und diese weiterhin aus Einsen bestehen,
// was der Bedeutung: "alle Frames belegt" entspricht.
// Daher wird die Initialisierung durch einen Konditional-Ausdruck realisiert, der gewährleistet, dass entweder
// um die Anzahl stellen aus der Modulo-Rechnung geshiftet wird, oder um volle 32 Stellen, falls die Rechnung 0 ergibt.
#define VMEM_LASTBMMASK (~0U << (VMEM_NFRAMES % VMEM_BITS_PER_BMWORD == 0 ? VMEM_BITS_PER_BMWORD : (VMEM_NFRAMES % VMEM_BITS_PER_BMWORD)))

#define VMEM_BMSIZE     ((VMEM_NFRAMES - 1) / VMEM_BITS_PER_BMWORD + 1)

/* Page Table */
#define PTF_PRESENT     1
#define PTF_DIRTY       2       /* store: need to write */
#define PTF_USED        4       /* For clock algo only */

struct pt_entry {
    int flags;                  /* see defines above */
    int frame;                  /* Frame idx */
    int count;                  /* For LRU algo */
};

struct vmem_adm_struct {
    int size;
    pid_t mmanage_pid;
    int shm_id;
    sem_t sema;                 /* Coordinate acces to shm */
    int req_pageno;             /* Number of requested page */
    int next_alloc_idx;         /* Next frame to allocate (FIFO, CLOCK) 
                                 */
    int pf_count;               /* Page fault counter */
    int g_count;                /* Global counter for LRU algo */
    Bmword bitmap[VMEM_BMSIZE]; /* 0 = free */
};

struct pt_struct {
    struct pt_entry entries[VMEM_NPAGES];
    int framepage[VMEM_NFRAMES];        /* pages on frame */
};

/* This is to be located in shared memory */
struct vmem_struct {
    struct vmem_adm_struct adm;
    struct pt_struct pt;
    int data[VMEM_NFRAMES * VMEM_PAGESIZE];
};

#define SHMSIZE (sizeof(struct vmem_struct))

#endif /* VMEM_H */
