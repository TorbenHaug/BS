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
	key_t shm_key = 0;
	int shm_id = -1;
	printf("PRE GET\n");

	shm_id = shmget(shm_key, SHMSIZE, 0664 |IPC_CREAT);

	//TODO: ?
	printf("PRE ALLOCATE\n");

	vmem = shmat(shm_id, NULL, 0);
}

/* Read from "virtual" address */
int vmem_read(int address){
	if(vmem == NULL){
		vm_init();
	}
	int res;

	int page = address / VMEM_PAGESIZE;
	int offset = address % VMEM_PAGESIZE;
	// Holen der Flags für die aktuelle Page (Interresant ist Present)
	int flags = vmem->pt.entries[page].flags;
	// Vergleichen, ob Present gesetzt ist
	int req_page_is_loaded = ((flags & PTF_PRESENT) == PTF_PRESENT);
	//falls nicht:
	if (!req_page_is_loaded) {
		vmem->adm.req_pageno = page;
		//MMange auffordern den seitenrahmen zu laden
		kill(vmem->adm.mmanage_pid, SIGUSR1);
		// auf MManage warten
		sem_wait(&vmem->adm.sema);
	}
	res = read_page(page, offset);
	return res;

}

/* Write data to "virtual" address */
void vmem_write(int address, int data){
	//TODO: vmem_write
	return;
}

