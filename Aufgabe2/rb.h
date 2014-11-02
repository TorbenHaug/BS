
#ifndef RB_H_
#define RB_H_
	#include <stdio.h>
	#include <stdlib.h>
	#include <curses.h>
	#include <unistd.h>
	#include <pthread.h>
	#define MAX 16



	typedef struct {
		int buffer[MAX];
		int *p_in;
		int *p_out;
		int count;
	}rb;

	#ifndef MODUL0
		#define EXTERN extern
		EXTERN pthread_mutex_t rb_mutex;

		EXTERN pthread_cond_t not_empty_condvar;
		EXTERN pthread_cond_t not_full_condvar;

		EXTERN pthread_cond_t prod_1_restart;
		EXTERN pthread_cond_t prod_2_restart;
		EXTERN pthread_cond_t cons_restart;
		EXTERN int prod_1_stopped;
		EXTERN int prod_2_stopped;
		EXTERN int cons_stopped;
		EXTERN rb *p_rb;
		EXTERN pthread_t threads[4];
		EXTERN int thread_id[4];
	#endif
	void* p_1_w(void *pid);
	void* p_2_w(void *pid);
	#ifndef MAX
		#define MAX 16
	#endif
	#ifndef p_start
		#define p_start (int *)(p_rb -> buffer)
	#endif
	#ifndef p_end
		#define p_end (int *)((p_rb -> buffer) + MAX-1)
	#endif
#endif // RB_H_
