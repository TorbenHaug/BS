#include "producer.h"
#include "rb.h"

void* producer(void *pid, pthread_cond_t *restart, int *prod_stopped, char beginn, char end, int *sleep_time);

void* p_1_w(void *pid){
	return producer(pid, &prod_1_restart, &prod_1_stopped, 'a', 'z', &producer1_sleep);
}

void* p_2_w(void *pid){
	return producer(pid,&prod_2_restart, &prod_2_stopped, 'A', 'Z', &producer2_sleep);
}

void* producer(void *pid, pthread_cond_t *restart, int *prod_stopped, char beginn, char end, int *sleep_time){
	int i = 0;
	int z_var = beginn - 1;

	printf("Producer started; %d: \n", *(int*)pid);

	while(1)
	{
		i++;
		// naechstes zu schreibendes Zeichen
		z_var++;
		// Zirkel garantieren: 'a' = 97, 'z' = 122
		if (z_var > end){
			z_var = beginn;
		}
		//mutex locken
		pthread_testcancel();
		pthread_mutex_lock(&rb_mutex);

		//prüfen, ob der Speicher voll ist, oder ob der producer angehalten ist
		while((p_rb -> p_in == p_rb -> p_out && p_rb -> count == MAX) || *prod_stopped)
		{
			// prüfen, welche von beiden bedingungen zutrifft
			if (p_rb -> p_in == p_rb -> p_out && p_rb -> count == MAX){
				if (verbose)
					printf("Producer %d: RingBuffer full.\n", *(int*)pid);
				pthread_cond_wait(&not_full_condvar, &rb_mutex);
				if (verbose)
					printf("Producer %d: RingBuffer no longer full.\n", *(int*)pid);
			}
			else if(*prod_stopped){
				if (verbose)
					printf("Producer %d: Stopped by user.\n", *(int*)pid);
				pthread_cond_wait(restart, &rb_mutex);
				if (verbose)
					printf("Producer %d: Started by user.\n", *(int*)pid);

			}
		}

		// in den puffer Schreiben
		if (verbose)
				printf("Producer %d: Added sign '%c' to Ringbuffer.\n", *(int*)pid, z_var);
		//printf("lege in puffer: %c", z_var);
		*(p_rb -> p_in) = z_var;
		// input zeiger vorschieben
		(p_rb -> p_in)++;
		// zirkulien des Zeigers garantieren
		if((p_rb -> p_in) > p_end)
		{
			p_rb -> p_in = p_start;
		}
		// belegung des Puffers erhöhen
		(p_rb -> count)++;
		//printf("Producer : %d: added '%c' to the buffer.", *(int*)pid, z_var);
		// Signal an den Consumer, dass der Puffer gefüllt ist
		if(p_rb -> count != 0)
		{
			pthread_cond_signal(&not_empty_condvar);
		}
		// Mutex freigeben
		pthread_mutex_unlock(&rb_mutex);
		// Thread 3 Secunden anhalten
		sleep(*sleep_time);
	}
	return NULL;
}
