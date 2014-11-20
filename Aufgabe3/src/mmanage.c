/* Description: Memory Manager BSP3*/
/* Prof. Dr. Wolfgang Fohl, HAW Hamburg */
/* Winter 2010/2011
 * 
 * This is the memory manager process that
 * works together with the vmaccess process to
 * mimic virtual memory management.
 *
 * The memory manager process will be invoked
 * via a SIGUSR1 signal. It maintains the page table
 * and provides the data pages in shared memory
 *
 * This process is initiating the shared memory, so
 * it has to be started prior to the vmaccess process
 *
 * TODO:
 * currently nothing
 * */

#include "mmanage.h"
#include <stdio.h>

struct vmem_struct *vmem = NULL;
FILE *pagefile = NULL;
FILE *logfile = NULL;
int signal_number = 0;          /* Received signal */

int
main(void)
{
	//setbuf(stdout, NULL);
    struct sigaction sigact;

    /* Init pagefile */
    init_pagefile(MMANAGE_PFNAME);
    if(!pagefile) {
        perror("Error creating pagefile");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "Pagefile successfully created\n");
    }
#endif /* DEBUG_MESSAGES */

    /* Open logfile */
    logfile = fopen(MMANAGE_LOGFNAME, "w");
    if(!logfile) {
        perror("Error creating logfile");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "Logfile successfully created\n");
    }
#endif /* DEBUG_MESSAGES */

    /* Create shared memory and init vmem structure */
    vmem_init();
    if(!vmem) {
        perror("Error initialising vmem");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "vmem successfully created\n");
    }
#endif /* DEBUG_MESSAGES */

    /* Setup signal handler */
    /* Handler for USR1 */
    sigact.sa_handler = sighandler;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    if(sigaction(SIGUSR1, &sigact, NULL) == -1) {
        perror("Error installing signal handler for USR1");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "USR1 handler successfully installed\n");
    }
#endif /* DEBUG_MESSAGES */

    if(sigaction(SIGUSR2, &sigact, NULL) == -1) {
        perror("Error installing signal handler for USR2");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "USR2 handler successfully installed\n");
    }
#endif /* DEBUG_MESSAGES */

    if(sigaction(SIGINT, &sigact, NULL) == -1) {
        perror("Error installing signal handler for INT");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "INT handler successfully installed\n");
    }
#endif /* DEBUG_MESSAGES */

    /* Signal processing loop */
    while(1) {
        signal_number = 0;
        pause();
        if(signal_number == SIGUSR1) {  /* Page fault */
#ifdef DEBUG_MESSAGES
            fprintf(stderr, "Processed SIGUSR1\n");
#endif /* DEBUG_MESSAGES */
            signal_number = 0;
        }
        else if(signal_number == SIGUSR2) {     /* PT dump */
#ifdef DEBUG_MESSAGES
            fprintf(stderr, "Processed SIGUSR2\n");
#endif /* DEBUG_MESSAGES */
            signal_number = 0;
        }
        else if(signal_number == SIGINT) {
#ifdef DEBUG_MESSAGES
            fprintf(stderr, "Processed SIGINT\n");
#endif /* DEBUG_MESSAGES */
        }
    }


    return 0;
}

/* Please DO keep this function unmodified! */
void
logger(struct logevent le)
{
    fprintf(logfile, "Page fault %10d, Global count %10d:\n"
            "Removed: %10d, Allocated: %10d, Frame: %10d\n",
            le.pf_count, le.g_count,
            le.replaced_page, le.req_pageno, le.alloc_frame);
    fflush(logfile);
}

void
vmem_init(void)
{
	key_t shm_key = 23456;
	int shm_id = -1;

	shm_id = shmget(shm_key, SHMSIZE, 0664 |IPC_CREAT);
	if(shm_id == -1){
		perror("Error initialising shmget");
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "Sharedmemory successfully allocated\n");
    }
#endif /* DEBUG_MESSAGES */

	vmem = shmat(shm_id, NULL, 0);
	if(vmem == (char *)-1){
		perror("Error initialising shmat");
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "Sharedmemory successfully attached to local memory\n");
    }
#endif /* DEBUG_MESSAGES */

	vmem->adm.size = VMEM_NPAGES * VMEM_PAGESIZE;
	vmem->adm.mmanage_pid = getpid();
	vmem->adm.shm_id = shm_id;

	if (sem_init(&(vmem->adm.sema), 1, 0) == - 1){
		perror("Error initialising sem_init");
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "Semaphore successfully installed\n");
    }
#endif /* DEBUG_MESSAGES */
	return;
}

void
init_pagefile(const char *pfname)
{
	int itemNumbers = VMEM_NPAGES * VMEM_PAGESIZE;
	int data[itemNumbers];

	srand(SEED);

	int i;

	for(i = 0; i < itemNumbers; i++) {
		data[i] = rand() % 1000;
	}

	pagefile = fopen(pfname, "w+b");
	if(!pagefile) {
		perror("Error creating pagefile");
		exit(EXIT_FAILURE);
	}

	int write_result = fwrite(data, sizeof(int), itemNumbers, pagefile);
	if(!write_result) {
		perror("Error creating pagefile");
		exit(EXIT_FAILURE);
	}

#ifdef DEBUG_MESSAGES
	fprintf(stderr, "Pagefile Success");
#endif
}

void sighandler(int signo){
	signal_number = signo;
	if(signo == SIGUSR1){
		allocate_page();
	}
	else if(signo == SIGUSR2){
		dump_pt();
	}
	else if(signo == SIGINT){
		cleanup();

		exit(EXIT_SUCCESS);
	}
}
void cleanup(){
	//close opened files
	if (fclose(pagefile)){
		perror("Error closing pagefile");
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "Pagefile closed\n");
    }
#endif /* DEBUG_MESSAGES */
	if (fclose(logfile)){
		perror("Error closing logfile");
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "Logfile closed\n");
    }
#endif /* DEBUG_MESSAGES */
	// zerstöre den Shared Memory
	int shm_id = vmem->adm.shm_id;
	while(!vmem){
		shmctl(shm_id, IPC_RMID, 0);
		if(!vmem){
			fprintf(stderr, "Shared memory can't be destroyed, please close all other processes using it\n");
			sleep(1);
		}
	}
#ifdef DEBUG_MESSAGES
		fprintf(stderr, "Shared memory successfully destroyed");
        fprintf(stderr, "Programm erfolgreich beendet\n");
#endif /* DEBUG_MESSAGES */
}
void allocate_page(void){
	int req_pageno = vmem->adm.req_pageno;
	int frame = VOID_IDX;
	int page_removed_idx = VOID_IDX;
	/* Page not allocated? */
	if(vmem->pt.entries[req_pageno].flags & PTF_PRESENT){
		return; //nichts zu tun, keiner wes warum diese funktion gerufen wurde?
		//Todo: ?
	}
	/* Free frames? */
	frame = search_bitmap();
	if(frame != VOID_IDX) {
		fprintf(stderr, "Found free frame no %d, allocating page\n", frame);
	}
	/* end if FRAME_VOID */ /* No free frames: Which page to remove? */
	else{
		frame = find_remove_frame();
		page_removed_idx = vmem->pt.framepage[frame];
		/* Store page to be removed and clear present-bit */
		if(vmem->pt.entries[page_removed_idx].flags & PTF_DIRTY) {
			store_page(page_removed_idx);
		}

		vmem->pt.entries[page_removed_idx].flags &= ~PTF_PRESENT;
	}
	/* Load new page */
	update_pt(frame);
	fetch_page(vmem->adm.req_pageno);

	/* Update page fault counter */
	vmem->adm.pf_count++;
#ifdef DEBUG_MESSAGES
        fprintf(stderr, "Allocated Page %d to %d", req_pageno, frame);
#endif /* DEBUG_MESSAGES */
	/* Log action */
	struct logevent le;
	le.req_pageno = vmem->adm.req_pageno;
	le.replaced_page = page_removed_idx;
	le.alloc_frame = frame;
	le.g_count = vmem->adm.g_count;
	le.pf_count = vmem->adm.pf_count;
	logger(le);
	/* Unblock application */
	sem_post(&(vmem->adm.sema));
}

// Prints a dump of the page table
void dump_pt(void) {
	fprintf(stderr, "Dump pagetable\n\n");

	fprintf(stderr, "admin struct\n");
	fprintf(stderr, "------------\n");
	fprintf(stderr, "size: %d, pf_count: %d\n", vmem->adm.size, vmem->adm.pf_count);
	fprintf(stderr, "req_pageno: %d, next_alloc_idx: %d\n", vmem->adm.req_pageno, vmem->adm.next_alloc_idx);
	fprintf(stderr, "g_count: %d\n\n", vmem->adm.g_count);

	fprintf(stderr, "data\n");
	fprintf(stderr, "----\n");
	int i;
	for(i = 0; i < (VMEM_NFRAMES * VMEM_PAGESIZE); i++) {
		fprintf(stderr, "idx %d: %d\n", i, vmem->data[i]);
	}
}

void fetch_page(int pt_idx){
	int offset = pt_idx * sizeof(int) * VMEM_PAGESIZE;
	int frame = vmem->pt.entries[pt_idx].frame;
	int *pstart = &(vmem->data[frame * VMEM_PAGESIZE]);
	/* fseek: change the file position indicator for the specified stream*/
	if(fseek(pagefile, offset, SEEK_SET) == -1) {
		perror("Positioning in pagefile failed! ");
		exit(EXIT_FAILURE);
	}
	fread(pstart, sizeof(int), VMEM_PAGESIZE, pagefile);
}

void store_page(int pt_idx){
	int offset = pt_idx * sizeof(int) * VMEM_PAGESIZE;
	int frame = vmem->pt.entries[pt_idx].frame;
	int *pstart = &vmem->data[frame * VMEM_PAGESIZE];
	if(fseek(pagefile, offset, SEEK_SET) == -1){
		perror("Positioning in pagefile failed! ");
		exit(EXIT_FAILURE);
	}

	if(!fwrite(pstart, sizeof(int), VMEM_PAGESIZE, pagefile)){
		perror("Writing in pagefile failed! ");
	exit(EXIT_FAILURE);
	}
}

void update_pt(int frame){
	int page_idx = vmem->adm.req_pageno;
	int bm_idx = frame / VMEM_BITS_PER_BMWORD;
	int bit = frame % VMEM_BITS_PER_BMWORD;
	/* Update bitmap */
	vmem->adm.bitmap[bm_idx] |= (1U << bit);
	/* Increment of next_alloc_idx */
	vmem->adm.next_alloc_idx = (vmem->adm.next_alloc_idx + 1) % VMEM_NFRAMES;
	/* Update framepage */
	vmem->pt.framepage[frame] = page_idx;
	/* Update pt_entry */
	vmem->pt.entries[page_idx].flags |= PTF_USED | PTF_PRESENT;
	vmem->pt.entries[page_idx].flags &= ~PTF_DIRTY;
	vmem->pt.entries[page_idx].frame = frame;
	vmem->pt.entries[page_idx].count = vmem->adm.g_count;
}

int find_remove_frame(void){
	int remove_frame = VOID_IDX;
	switch (VMEM_ALGO) {
		case VMEM_ALGO_LRU:
			remove_frame = find_remove_lru();
			break;
		case VMEM_ALGO_CLOCK:
			remove_frame = find_remove_clock();
			break;
		case VMEM_ALGO_FIFO:
		default:
			remove_frame = find_remove_fifo();
			break;
	}
	return remove_frame;
}

int find_remove_clock(void){
	int remove_frame = vmem->adm.next_alloc_idx;
	int frame = remove_frame;
	int page;
	while(1) {
		page = vmem->pt.framepage[frame];
		if(vmem->pt.entries[page].flags & PTF_USED) {
			vmem->pt.entries[page].flags &= ~PTF_USED;
			/* clear used flag*/
			frame = (frame + 1) % VMEM_NFRAMES;
		}
		else { /* frame not marked as used */
			remove_frame = frame;
			break;
		}
	}
	/* end while */
	vmem->adm.next_alloc_idx = remove_frame ;
	return remove_frame;
}
int find_remove_fifo(void){
	// holen des nächsten zu beschreibenden Hauptspeicher Frames
	int remove_frame = vmem->adm.next_alloc_idx;
	return remove_frame;

}
int find_remove_lru(void){
	int i;
	int remove_frame = VOID_IDX;
	// smallest_count auf infinity setzen
	unsigned int smallest_count = -1;
	for(i = 0; i < VMEM_NFRAMES; i++){
		int page = vmem->pt.framepage[i];
		if(vmem->pt.entries[page].count < smallest_count){
			smallest_count = vmem->pt.entries[page].count;
			remove_frame = i;
		}
	}
	return remove_frame;
}
//????????????????????????
int search_bitmap(void){
	int i;
	int free_bit = VOID_IDX;
	for(i = 0; i < VMEM_BMSIZE; i++) {
		Bmword bitmap = vmem->adm.bitmap[i];
		Bmword mask = (i == (VMEM_BMSIZE - 1) ? VMEM_LASTBMMASK : 0);
		free_bit = find_free_bit(bitmap, mask);
		if(free_bit != VOID_IDX) {
			break;
		}
	}
	/* end for i */
	return free_bit;
}

int find_free_bit(Bmword bmword, Bmword mask){
	int bit = VOID_IDX;
	Bmword bitmask = 1; /* Mask bmword */
	bmword |= mask;
	for(bit = 0; bit < VMEM_BITS_PER_BMWORD; bit++) {
		if(!(bmword & bitmask)) {
			break;
		}
		/* end if */
		bitmask <<= 1;
	} /* end for */
	return bit < VMEM_BITS_PER_BMWORD ? bit : VOID_IDX;
}
