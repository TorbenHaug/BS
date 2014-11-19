/*
 * vmaccess.c
 *
 *  Created on: 06.11.2014
 *      Author: torbenhaug
 */

#include "vmaccess.h"

struct vmem_struct *vmem = NULL;

/* Connect to shared memory (key from vmem.h) */
void vm_init(void){
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
}

/* Read from "virtual" address */
int vmem_read(int address){

	//PhysAddr holen
	int phys_addr = phys_index(address,0);
	//Inhalt des Speichers zurückgeben
	return vmem->data[phys_addr];

}

/* Write data to "virtual" address */
void vmem_write(int address, int data){
	//PhysAddr holen
	int phys_addr = phys_index(address,1);
	// Inhalt des Speichers verändern
	vmem->data[phys_addr] = data;
	return;
}

int phys_index(int address, int write){
	if(vmem == NULL){
		vm_init();
	}

	// Errechnen der Page über die Adresse
	// dadurch, dass die führenden Bits der Virtuellen Adresse
	// die Seitennummer angeben, können wir einfach Ganzzahlig
	// durch die größe der Page teilen
	int page = address / VMEM_PAGESIZE;
	// Somit ergibt sich das Offset aus dem Rest
	int offset = address % VMEM_PAGESIZE;
	// Holen der Flags für die aktuelle Page (Interresant ist Present)
	int flags = vmem->pt.entries[page].flags;
	// Vergleichen, ob Present gesetzt ist durch Logische und XX1 & 1 == 1 || XX0 & 1 == 0
	int req_page_is_loaded = ((flags & PTF_PRESENT) == PTF_PRESENT);
	//falls nicht:

	if (!req_page_is_loaded) {
		// Speichern der Page No für die übergabe an Mmanage
		vmem->adm.req_pageno = page;
		//MMange auffordern den seitenrahmen zu laden
		kill(vmem->adm.mmanage_pid, SIGUSR1);
		// auf MManage warten
		sem_wait(&vmem->adm.sema);
	}
	// used bit setzen
	vmem->pt.entries[page].flags |= PTF_USED;
	//Prüfn ob eine schreiboperation stattfindet
	if(write){
		//Dirty bit setzen
		vmem->pt.entries[page].flags |= PTF_DIRTY;
	}
	// neuen index (im PhysMem) errechen
	return (vmem->pt.entries[page].frame * VMEM_PAGESIZE) + offset;
}

