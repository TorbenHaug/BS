/*
 * vmaccess.c
 *
 *  Created on: 17.11.2014
 *      Author: torbenhaug
*/

#include "vmaccess.h"

struct vmem_struct *vmem = NULL;

// Connect to shared memory (key from vmem.h)
void vm_init(void){
  key_t shm_key = 23456;
  int shm_id = -1;

  shm_id = shmget(shm_key, SHMSIZE, 0664 |IPC_CREAT);
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

// Read from "virtual" address
int vmem_read(int address){

  //PhysAddr holen
  int phys_addr = phys_index(address,0);
  //Inhalt des Speichers zurückgeben
  return vmem->data[phys_addr];

}

// Write data to "virtual" address
void vmem_write(int address, int data){
  // Physikalische Adresse holen
  int phys_addr = phys_index(address,1);
  // Inhalt des Speichers verändern
  vmem->data[phys_addr] = data;
  return;
}

// 
int phys_index(int address, int write){
  if(vmem == NULL){
    vm_init();
  }

  // Ermittlung der Page über die Adresse.
  // Dadurch, dass die führenden Bits der virtuellen Adresse
  // die Seitennummer angeben, können wir eine Ganzzahldivision
  // durch die Größe einer Page teilen
  int page = address / VMEM_PAGESIZE;
  
  // Somit ergibt sich das Offset aus dem Rest
  int offset = address % VMEM_PAGESIZE;
  
  // Holen der Flags für die aktuelle Page (Interresant ist Present)
  int flags = vmem->pt.entries[page].flags;
  
  // Mit logischem UND vergleichen, ob Present-Bit gesetzt ist (XX1 & 1 == 1 || XX0 & 1 == 0)
  int req_page_is_loaded = ((flags & PTF_PRESENT) == PTF_PRESENT);

  // Falls die angeforderte Page nicht geladen ist
  if(!req_page_is_loaded){
    // wird die Nummer der angeforderten Page in der Admin Strukt., zur Übergabe an 'mmanage' gespeichert
    vmem->adm.req_pageno = page;
    // und 'mmanage' zum Laden der Page aufgefordert
    kill(vmem->adm.mmanage_pid, SIGUSR1);
    // und die Prozedur blockiert an dieser Stelle, um auf das Signal von 'mmanage',
    // zu warten, dass der Lagevorgang abgeschlossen wurde
    sem_wait(&vmem->adm.sema); // Signal wird über Semaphor erwartet
  }
  // Used-Bit setzen
  vmem->pt.entries[page].flags |= PTF_USED;
  // Prüfen, ob eine Schreiboperation durchgeführt wurde
  if(write){
    // Dirty-Bit setzen
    vmem->pt.entries[page].flags |= PTF_DIRTY;
  }

  // Inkrementiere den Global-Count g_count (Zugriff auf den Speichr ist erfolgt) - relevant für LRU
  vmem->adm.g_count++;
  // Merke den Globalcount-Wert im Attr. 'count' für den aktuellen Frame - relevant für LRU
  vmem->pt.entries[page].count = vmem->adm.g_count;

  // Neuen index (im PhysMem) errechen
  return (vmem->pt.entries[page].frame * VMEM_PAGESIZE) + offset;
}

