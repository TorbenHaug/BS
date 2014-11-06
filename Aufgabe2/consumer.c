#include "consumer.h"

void* consumer(void *pid)
{
	int i = 0;
	printf("Consumer started; %d: \n", *(int*)pid);
	while(1)
	{
		i++;
		pthread_mutex_lock(&rb_mutex);
		// Überprüfen, ob alle Runbedingungen zutreffen.
		while(p_rb -> count == 0 || cons_stopped)
		{
			// Überprüfen welche Runbedingung nich zutrifft:
			if (p_rb -> count == 0 ){
				if (verbose)
					printf("Consumer: Ringbuffer empty.\n");
				pthread_cond_wait(&not_empty_condvar, &rb_mutex);
				if (verbose)
					printf("Consumer: Ringbuffer not empty longer.\n");
			}
			else if(cons_stopped){
				if (verbose)
					printf("Consumer: Stopped by user.\n");
				pthread_cond_wait(&cons_restart, &rb_mutex);
				if (verbose)
					printf("Consumer: Started by user.\n");
			}
		}
		(p_rb -> count)--;
		// Ältestes Zeichen ausgeben
		printf("%c:", *(p_rb -> p_out));

		// Umbruch, falls verbose mode an
		if (verbose)
			printf("\n");

		fflush(stdout);
		//Prüfen, ob es das 30gste zeichen war
		if ((i % 30) == 0){
			printf("\n");
			i = 0;
		}
		(p_rb -> p_out)++;
		if((p_rb -> p_out) > p_end)
		{
			p_rb -> p_out = p_start;
		}
		if(p_rb -> count <= MAX)
		{
			//Buffer nicht voll --> signalisieren
			pthread_cond_signal(&not_full_condvar);
		}
		pthread_mutex_unlock(&rb_mutex);
		sleep(consumer_sleep);
	}
	return NULL;
}
