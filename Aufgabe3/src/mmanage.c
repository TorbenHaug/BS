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
int signal_number = 0;          // Zeigt an, welches Signal empfangen wurde

int
main(void)
{
	// setbuf(stdout, NULL);
    struct sigaction sigact;

    // Pagefile initialisieren
    init_pagefile(MMANAGE_PFNAME);
    if(!pagefile) {
        perror("Error creating pagefile");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "Pagefile successfully created\n");
    }
#endif//DEBUG_MESSAGES 

    // Logfile öffnen
    logfile = fopen(MMANAGE_LOGFNAME, "w");
    if(!logfile) {
        perror("Error creating logfile");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "Logfile successfully created\n");
    }
#endif//DEBUG_MESSAGES 

    // Shared Memory erzeugen und VMEM Struktur initialisieren
    vmem_init();
    if(!vmem) {
        perror("Error initialising vmem");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "vmem successfully created\n");
    }
#endif//DEBUG_MESSAGES 

    // Signal Handler einrichten
    // Handler für USR1 
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
#endif//DEBUG_MESSAGES 

    if(sigaction(SIGUSR2, &sigact, NULL) == -1) {
        perror("Error installing signal handler for USR2");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "USR2 handler successfully installed\n");
    }
#endif//DEBUG_MESSAGES 

    if(sigaction(SIGINT, &sigact, NULL) == -1) {
        perror("Error installing signal handler for INT");
        exit(EXIT_FAILURE);
    }
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "INT handler successfully installed\n");
    }
#endif//DEBUG_MESSAGES 

    // Schleife, zur Anzeige der eingetroffnen Signale
    while(1) {
        signal_number = 0;
        pause();
        if(signal_number == SIGUSR1) {  // Ein PageFault ist aufgetreten
#ifdef DEBUG_MESSAGES
            fprintf(stderr, "Processed SIGUSR1\n");
#endif//DEBUG_MESSAGES 
            signal_number = 0; // Signalnummer zurücksetzen
        }
        else if(signal_number == SIGUSR2) {  // Es wurde der PageTable ausgegeben
#ifdef DEBUG_MESSAGES
            fprintf(stderr, "Processed SIGUSR2\n");
#endif//DEBUG_MESSAGES 
            signal_number = 0;
        }
        else if(signal_number == SIGINT) {
#ifdef DEBUG_MESSAGES
            fprintf(stderr, "Processed SIGINT\n");
#endif//DEBUG_MESSAGES 
        }
    }
    return 0;
}

// Please DO keep this function unmodified! 
void
logger(struct logevent le)
{
    fprintf(logfile, "Page fault %10d, Global count %10d:\n"
            "Removed: %10d, Allocated: %10d, Frame: %10d\n",
            le.pf_count, le.g_count,
            le.replaced_page, le.req_pageno, le.alloc_frame);
    fflush(logfile);
}

// Initialisierung der VMEM Struktur
void
vmem_init(void)
{
	// SM-Schlüssel festlegen
  key_t shm_key = 23456;
	int shm_id = -1;

  // SM anfordern und gelieferte ID merken
	shm_id = shmget(shm_key, SHMSIZE, 0664 |IPC_CREAT);
	if(shm_id == -1){
		perror("Error initialising shmget");
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "Sharedmemory successfully allocated\n");
    }
#endif//DEBUG_MESSAGES 

  // Strukturwerte des SM in vmem laden
	vmem = shmat(shm_id, NULL, 0);
	if(vmem == (char *)-1){
		perror("Error initialising shmat");
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "Sharedmemory successfully attached to local memory\n");
    }
#endif//DEBUG_MESSAGES 

  // Zusätzliche Werte in der Admin-Struktur setzen
	vmem->adm.size = VMEM_NPAGES * VMEM_PAGESIZE;
	vmem->adm.mmanage_pid = getpid();
	vmem->adm.shm_id = shm_id;

  // Semaphor initialisieren
	if (sem_init(&(vmem->adm.sema), 1, 0) == - 1){
		perror("Error initialising sem_init");
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "Semaphore successfully installed\n");
    }
#endif//DEBUG_MESSAGES 
	return;
}

// Initialisierung der Pagefile
void
init_pagefile(const char *pfname)
{
	int itemNumbers = VMEM_NPAGES * VMEM_PAGESIZE;
	int data[itemNumbers];

	srand(SEED); // Zufallszahlen-Seed randomisieren

	int i;

  // Auslagerungsdatei mit zufälligen Werten...
	for(i = 0; i < itemNumbers; i++) {
		data[i] = rand() % 1000; // ...zwischen 0-999 füllen
	}

  // Dateistream öffnen...
	pagefile = fopen(pfname, "w+b");
	if(!pagefile) {
		perror("Error creating pagefile");
		exit(EXIT_FAILURE);
	}

  // ...und Daten ausschreiben
	int write_result = fwrite(data, sizeof(int), itemNumbers, pagefile);
	if(!write_result) {
		perror("Error creating pagefile");
		exit(EXIT_FAILURE);
	}

#ifdef DEBUG_MESSAGES
	fprintf(stderr, "Pagefile Success");
#endif
}

// Signal Handler zur Verarbeitung eines Signals
void
sighandler(int signo)
{
	signal_number = signo; // Nummer des empfangenen Signals modul-global anzeigen
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

// Funktion zum Aufräumen bei Beendigung des Programms
void cleanup(){
	// Geöffnete Dateien schließen
	if (fclose(pagefile)){
		perror("Error closing pagefile");
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "Pagefile closed\n");
    }
#endif//DEBUG_MESSAGES 
	if (fclose(logfile)){
		perror("Error closing logfile");
		exit(EXIT_FAILURE);
	}
#ifdef DEBUG_MESSAGES
    else {
        fprintf(stderr, "Logfile closed\n");
    }
#endif//DEBUG_MESSAGES 
	// Shared Memory zerstören
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
#endif//DEBUG_MESSAGES 
}

// Zum Laden der angeforderten Page
void allocate_page(void){
	int req_pageno = vmem->adm.req_pageno;
	int frame = VOID_IDX;
	int page_removed_idx = VOID_IDX;
	
  // Prüfen, ob angeforderte Page geladen ist
	if(vmem->pt.entries[req_pageno].flags & PTF_PRESENT){
		return; // Abbrechen, da Page bereits geladen
	}
  // ...sonst
	// Bitmap nach freiem Frame durchsuchen
	frame = search_bitmap();
  if(frame != VOID_IDX){
		fprintf(stderr, "Found free frame no %d, allocating page\n", frame);
	}
  // Falls kein freier Frame verfügbar
  else{
    // Frame ermitteln, der geleert werden kann,
    // um anschließend die angeforderte Seite dort hinein laden zu können
		frame = find_remove_frame();
		page_removed_idx = vmem->pt.framepage[frame];
		
    // Falls Page modifiziert wurde
		if(vmem->pt.entries[page_removed_idx].flags & PTF_DIRTY) {
			store_page(page_removed_idx); // Page in Auslagerungsdatei sichern / aktualisieren
		}
    // Present-Bit zurücksetzen
		vmem->pt.entries[page_removed_idx].flags &= ~PTF_PRESENT;
	}
  
	// Neue Page laden
	update_pt(frame);
	fetch_page(vmem->adm.req_pageno);

	// PageFault Zählen inkrementieren
	vmem->adm.pf_count++;
  
#ifdef DEBUG_MESSAGES
        fprintf(stderr, "Allocated Page %d to %d", req_pageno, frame);
#endif//DEBUG_MESSAGES

	// Ereignis loggen
	struct logevent le;
	le.req_pageno = vmem->adm.req_pageno;
	le.replaced_page = page_removed_idx;
	le.alloc_frame = frame;
	le.g_count = vmem->adm.g_count;
	le.pf_count = vmem->adm.pf_count;
	logger(le);
  
	// Anwendung freigeben 
	sem_post(&(vmem->adm.sema));
}

// Gibt den Inhalt des PageTable's aus
void dump_pt(void) {
	fprintf(stderr, "Dump pagetable\n\n");

	fprintf(stderr, "admin struct\n");
	fprintf(stderr, "------------\n");
	fprintf(stderr, "size: %d, pf_count: %d\n", vmem->adm.size, vmem->adm.pf_count);
	fprintf(stderr, "req_pageno: %d, next_alloc_idx: %d\n", vmem->adm.req_pageno, vmem->adm.next_alloc_idx);
	fprintf(stderr, "g_count: %d\n\n", vmem->adm.g_count);

	fprintf(stderr, "data\n");
	fprintf(stderr, "----\n");
	int i; // Daten des Shared Memory ausgeben
	for(i = 0; i < (VMEM_NFRAMES * VMEM_PAGESIZE); i++) {
		fprintf(stderr, "idx %d: %d\n", i, vmem->data[i]);
	}
}

// Page aus Pagefile holen
void fetch_page(int pt_idx){
	int offset = pt_idx * sizeof(int) * VMEM_PAGESIZE;
	int frame = vmem->pt.entries[pt_idx].frame;
	int *pstart = &(vmem->data[frame * VMEM_PAGESIZE]);
  // fseek: Position des Zeigers in der Datei für den Inputstream setzen
	if(fseek(pagefile, offset, SEEK_SET) == -1) {
		perror("Positioning in pagefile failed! ");
		exit(EXIT_FAILURE);
	}
	fread(pstart, sizeof(int), VMEM_PAGESIZE, pagefile);
}

// Page in Pagefile auslagern
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

// Einträge des PageTables aktualisieren
void update_pt(int frame){
	int page_idx = vmem->adm.req_pageno;
	int bm_idx = frame / VMEM_BITS_PER_BMWORD;
	int bit = frame % VMEM_BITS_PER_BMWORD;
	// Update bitmap 
	vmem->adm.bitmap[bm_idx] |= (1U << bit);
	// Inkrementiere next_alloc_idx (für Seitenersetzungsalgorithmen relevant)
	vmem->adm.next_alloc_idx = (vmem->adm.next_alloc_idx + 1) % VMEM_NFRAMES;
	// Update framepage 
	vmem->pt.framepage[frame] = page_idx;
	// Update pt_entry 
	vmem->pt.entries[page_idx].flags |= PTF_USED | PTF_PRESENT;
	vmem->pt.entries[page_idx].flags &= ~PTF_DIRTY;
	vmem->pt.entries[page_idx].frame = frame;
	vmem->pt.entries[page_idx].count = vmem->adm.g_count;
}

// Zur Findung eines Frames, der geleert werden kann,
// um eine angeforderte Seite dort hineinladen zu können
int find_remove_frame(void){
	int remove_frame = VOID_IDX;
	// Anhand des gesetzen Compiler-Flags entscheiden
  // welcher Ersetzungsalgorithmus verwendet werden soll
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

// CLOCK Seitenersetzungsalgorithmus
int find_remove_clock(void){
	int remove_frame = vmem->adm.next_alloc_idx;
	int frame = remove_frame;
	int page;
  
  // Den "Uhrzeiger" solange weiterstellen, bis ein Frame gefunden wird
  // der gerade nicht benutzt wird (Frame mit gelöschtem Used-Bit)
	while(1) {
		page = vmem->pt.framepage[frame];
		if(vmem->pt.entries[page].flags & PTF_USED) {
			// Used-Bit löschen
      vmem->pt.entries[page].flags &= ~PTF_USED;
      // "Uhrzeiger" weiterdrehen
			frame = (frame + 1) % VMEM_NFRAMES;
		}
		else { // Used-Bit des Frames ist nicht gesetzt
           // -> Frame wird in diesem Moment nicht benutzt
			remove_frame = frame;
			break;
		}
	}
	// end while 
	vmem->adm.next_alloc_idx = remove_frame ;
	return remove_frame;
}

// FIFO Seitenersetzungsalgorithmus
int find_remove_fifo(void){
	// Nummer des Frames, der als nächstes in der 'Schlange' steht, holen
	int remove_frame = vmem->adm.next_alloc_idx;
	return remove_frame;
}

// LRU (Least-Recently-Used) Seitenersetzungsalgorithmus
int find_remove_lru(void){
	int i;
	int remove_frame = VOID_IDX;
	// smallest_count auf 'unendlich' setzen
	unsigned int smallest_count = -1;
  
	// Nach dem Frame suchen, dessen Benutzung am längsten zurückliegt 
  for(i = 0; i < VMEM_NFRAMES; i++){
		int page = vmem->pt.framepage[i];
    if(vmem->pt.entries[page].count < smallest_count){
			smallest_count = vmem->pt.entries[page].count;
			// Nummer des Frames mit dem, bis hier hin, "kleinsten" Zeitstempel setzen
      remove_frame = i;
		}
	}
	return remove_frame;
}

// Ermittlung einer freien Stelle in der Speicherbitmap
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
	// end for i 
	return free_bit;
}

// Hilfsfunktion zur Ermittlung einer freien Stelle in der Speicherbitmap
int find_free_bit(Bmword bmword, Bmword mask){
	int bit = VOID_IDX;
	Bmword bitmask = 1; // Mask bmword 
	bmword |= mask;
	for(bit = 0; bit < VMEM_BITS_PER_BMWORD; bit++) {
		if(!(bmword & bitmask)) {
			break;
		}
		// end if 
		bitmask <<= 1;
	} // end for 
	return bit < VMEM_BITS_PER_BMWORD ? bit : VOID_IDX;
}