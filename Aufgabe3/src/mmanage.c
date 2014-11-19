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
	printf("INIT\n");
    struct sigaction sigact;

    /* Init pagefile */
    init_pagefile(MMANAGE_PFNAME);
    printf("POST PAGEFILE CREATE\n");
    if(!pagefile) {
        perror("Error creating pagefile");
        exit(EXIT_FAILURE);
    }

    /* Open logfile */
    logfile = fopen(MMANAGE_LOGFNAME, "w");
    printf("POST LOGFILE\n");
    if(!logfile) {
        perror("Error creating logfile");
        exit(EXIT_FAILURE);
    }

    /* Create shared memory and init vmem structure */
    vmem_init();
    printf("POST INIT VMEM \n");
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

	shm_id =shmget(shm_key, SHMSIZE, 0664 |IPC_CREAT);
	if(shm_id == -1){
		perror("Error initialising shmget");
		exit(EXIT_FAILURE);
	}

	vmem = shmat(shm_id, NULL, 0);
	if(vmem == (char *)-1){
		perror("Error initialising shmat");
		exit(EXIT_FAILURE);
	}

	vmem->adm.size = VMEM_NPAGES * VMEM_PAGESIZE;
	vmem->adm.mmanage_pid = getpid();
	vmem->adm.shm_id = shm_id;

	if (sem_init(&(vmem->adm.sema), 1, 0) == - 1){
		perror("Error initialising sem_init");
		exit(EXIT_FAILURE);
	}

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
		// TODO Error
		perror("Error creating pagefile");
		exit(EXIT_FAILURE);
	}

	int write_result = fwrite(data, sizeof(int), itemNumbers, pagefile);
	if(!write_result) {
		// TODO Error
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
		//dump_pt();
	}
	else if(signo == SIGINT){
		//cleanup();
		//Todo: destroy shared memory
		exit(EXIT_SUCCESS);
	}
}

void allocate_page(void){
	int req_pageno = vmem->adm.req_pageno;
	int frame = VOID_IDX;
	int page_removed_idx = VOID_IDX;
	/* Page not allocated? */
	if(vmem->pt.entries[req_pageno].flags & PTF_PRESENT){
		//Todo: ?
	}
	/* Free frames? */
	frame = search_bitmap();
	if(frame != VOID_IDX) {
		fprintf(stderr, "Found free frame no %d, allocating page\n", frame);
		//update_pt(frame); //unnessary?
		//fetch_page(vmem->adm.req_pageno); //unnessary?
	}
	/* end if FRAME_VOID */ /* No free frames: Which page to remove? */
	else{
		frame = find_remove_frame();
		//Todo: ?
		page_removed_idx = vmem->pt.framepage[frame];
		//Todo: ?


		//Todo: ?
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
	/* Log action */
	//Todo: ?
	/* Unblock application */
	sem_post(&(vmem->adm.sema));
	//Todo: ?
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
	fseek(pagefile, offset, SEEK_SET);
	//Todo: ?
	fwrite(pstart, sizeof(int), VMEM_PAGESIZE, pagefile);
	//Todo: ?
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
	//Todo: no idea
	//vmem->pt.entries[page_idx].startcount = vmem->adm.g_count;
	vmem->pt.entries[page_idx].count = 0;
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
