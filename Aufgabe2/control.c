/*
 ============================================================================
 Name        : control.c
 Author      : Tim Hartig
 Version     : 0.1
 Description : Controller fuer die Tastureingaben-geleitete Threadsteuerung.
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <pthread.h>
#include "control.h"

extern pthread_cond_t   prod_1_restart;
extern pthread_cond_t   prod_2_restart;
extern pthread_cond_t   cons_restart;

extern int prod_1_stopped;
extern int prod_2_stopped;
extern int cons_stopped;

void *control(void *pid) {
	int run = 1;
	char code;

	do {
		code = getchar();
		switch (code) {
			case '1':
				// Producer 1 toggle
				if (!prod_1_stopped) {
					pthread_cond_signal(&prod_1_restart);
					prod_1_stopped = 0;
				} else {
					prod_1_stopped = 1;
				}
				break;
			case '2':
				// Producer 2 toggle
				if (!prod_2_stopped) {
					pthread_cond_signal(&prod_2_restart);
					prod_2_stopped = 0;
				} else {
					prod_2_stopped = 1;
				}
				break;
			case 'c':
			case 'C':
				// Consumer toggle
				if (!cons_stopped) {
					pthread_cond_signal(&cons_restart);
					cons_stopped = 0;
				} else {
					cons_stopped = 1;
				}
				break;
			case 'q':
			case 'Q':
				// Threads terminate
        run = 0;
				break;
			case 'h':
      // Show help
				usage();
				break;
			default:
				break;
		}

	} while (run);

	return NULL;
}


// Hilfe ausgeben
void usage() {

}
