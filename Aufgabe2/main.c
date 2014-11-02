#define MODULO
#include "main.h"
#include "consumer.h"
#include "producer.h"
#include "control.h"

pthread_mutex_t rb_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t not_empty_condvar = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full_condvar = PTHREAD_COND_INITIALIZER;
pthread_cond_t prod_1_restart = PTHREAD_COND_INITIALIZER;
pthread_cond_t prod_2_restart = PTHREAD_COND_INITIALIZER;
pthread_cond_t cons_restart = PTHREAD_COND_INITIALIZER;

int prod_1_stopped = 0;
int prod_2_stopped = 0;
int cons_stopped = 0;

int thread_id[4] = {0,1,2,3};
pthread_t threads[4];

rb x = { {0}, NULL, NULL, 0};
rb *p_rb = &x;


int main(int argc, char* argv[])
{
	int i;
	printf("Start des Beispiels \n");
	//printf("Argumente verfuegbar: ARGC\n", 3*argc);
	
	// TODO
	
	p_rb -> p_in = p_start;
	p_rb -> p_out = p_start;
	p_rb -> count = 0;
	printf("Counter value %d\n", p_rb ->count);
	pthread_create(&threads[0], NULL, control, (void *)&thread_id[0]);
	pthread_create(&threads[1], NULL, p_1_w, (void *)&thread_id[1]);
	pthread_create(&threads[2], NULL, p_2_w, (void *) &thread_id[2]);
	pthread_create(&threads[3], NULL, consumer, (void *) &thread_id[3]);
	
	// TODO

	for(i = 0; i<4; i++)
		pthread_join(threads[i], NULL);
		
	printf("Ende nach Join der Threads\n");
	return 0;
}
