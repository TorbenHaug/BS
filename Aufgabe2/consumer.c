#include "consumer.h"

void* consumer(void *pid)
{
	int i = 0;
	printf("Start Lesen; %d: \n", *(int*)pid);
	while(1)
	{
		i++;
		pthread_mutex_lock(&rb_mutex);
		//Überprüfen, ob alle Runbedingungen zutreffen.
		while(p_rb -> count == 0 || cons_stopped)
		{
			//Überprüfen welche Runbedingung nich zutrifft:
			if (p_rb -> count == 0 ){
				pthread_cond_wait(&not_empty_condvar, &rb_mutex);
			}
			else if(cons_stopped){
				pthread_cond_wait(&cons_restart, &rb_mutex);
			}
		}
		(p_rb -> count)--;
		// Ältestes Zeichen ausgeben
		printf("%c:", *(p_rb -> p_out));
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
			//Buffer nicht voll, signalisieren
			pthread_cond_signal(&not_full_condvar);
		}
		pthread_mutex_unlock(&rb_mutex);
		sleep(2);
	}
	return NULL;
}
