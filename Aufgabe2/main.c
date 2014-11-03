#define MODULO
#include "main.h"
#include "consumer.h"
#include "producer.h"
#include "control.h"
#include <sched.h>
#include <string.h>
#include <stdlib.h>

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

pthread_t 	threads[4];
void* 		thread_result[4];

rb x = { {0}, NULL, NULL, 0};
rb *p_rb = &x;
int verbose = 0;
int control_prio = 10;
int consumer_prio = 10;
int producer1_prio = 10;
int producer2_prio = 10;


void pthread_attr_creator(pthread_attr_t* attr, int prio);

int main(int argc, char* argv[])
{
	int j = 1;
	for(;j < (argc); j++){
		if (strcmp(argv[j], "-v") == 0){
			verbose = 1;}
		else if (strcmp(argv[j], "-cont") == 0)
			control_prio = atoi(argv[++j]);
		else if (strcmp(argv[j], "-cons") == 0)
			consumer_prio = atoi(argv[++j]);
		else if (strcmp(argv[j], "-p1") == 0)
			producer1_prio = atoi(argv[++j]);
		else if (strcmp(argv[j], "-p2") == 0)
			producer2_prio = atoi(argv[++j]);
	}
	

	printf("Consumer Prioritaet: %d\n",consumer_prio);
	printf("Controler Prioritaet: %d\n",control_prio);
	printf("Producer1 Prioritaet: %d\n",producer1_prio);
	printf("Producer2 Prioritaet: %d\n",producer2_prio);

	p_rb -> p_in = p_start;
	p_rb -> p_out = p_start;
	p_rb -> count = 0;
	printf("Counter value %d\n", p_rb -> count);


	pthread_attr_t attr1; // 1. attribute
	pthread_attr_t attr2; // 1. attribute
	pthread_attr_t attr3; // 1. attribute
	pthread_attr_t attr4; // 1. attribute
	pthread_attr_creator(&attr1, control_prio);
	pthread_attr_creator(&attr2, producer1_prio);
	pthread_attr_creator(&attr3, producer2_prio);
	pthread_attr_creator(&attr4, consumer_prio);


	pthread_create(&threads[0], &attr1, control, (void *)&thread_id[0]);
	pthread_create(&threads[1], &attr2, p_1_w, (void *)&thread_id[1]);
	pthread_create(&threads[2], &attr3, p_2_w, (void *) &thread_id[2]);
	pthread_create(&threads[3], &attr4, consumer, (void *) &thread_id[3]);

	int i;
	for(i = 0; i < 4; i++)
		pthread_join(threads[i], thread_result[i]);

	printf("Ende nach Join der Threads\n");
	return 0;
}

void pthread_attr_creator(pthread_attr_t* attr, int prio){
	struct sched_param my_prio;
	pthread_attr_init(attr); // 2.
	pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED); //Freigabe der Parameteraenderung
	pthread_attr_setschedpolicy(attr, SCHED_FIFO);
	my_prio.sched_priority = prio; // 4.
	pthread_attr_setschedparam(attr, &my_prio);
}
