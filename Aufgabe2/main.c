#define MODULO
#include "main.h"
#include "consumer.h"
#include "producer.h"
#include "control.h"
#include <sched.h>

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


int main(int argc, char* argv[])
{
	int i;
	printf("Start des Beispiels \n");
	//printf("Argumente verfuegbar: ARGC\n", 3*argc);
	
	p_rb -> p_in = p_start;
	p_rb -> p_out = p_start;
	p_rb -> count = 0;
	printf("Counter value %d\n", p_rb -> count);


	pthread_attr_t attr1; // 1. attribute
	struct sched_param my_prio1; // priority vgl. S 48

	pthread_attr_t attr2; // 1. attribute
	struct sched_param my_prio2; // priority vgl. S 48

	pthread_attr_t attr3; // 1. attribute
	struct sched_param my_prio3; // priority vgl. S 48

	pthread_attr_t attr4; // 1. attribute
	struct sched_param my_prio4; // priority vgl. S 48


	pthread_attr_init(&attr1); // 2.
	pthread_attr_setinheritsched(&attr1, PTHREAD_EXPLICIT_SCHED); //Freigabe der Parameteraenderung
	pthread_attr_setschedpolicy(&attr1, SCHED_FIFO);
	my_prio1.sched_priority = 10; // 4.
	pthread_attr_setschedparam(&attr1, &my_prio1);
	pthread_create(&threads[0], &attr1, control, (void *)&thread_id[0]);


	pthread_attr_init(&attr2); // 2.
	pthread_attr_setinheritsched(&attr2, PTHREAD_EXPLICIT_SCHED); //Freigabe der Parameteraenderung
	pthread_attr_setschedpolicy(&attr2, SCHED_FIFO);
	my_prio2.sched_priority = 10; // 4.
	pthread_attr_setschedparam(&attr2, &my_prio2);
	pthread_create(&threads[1], &attr2, p_1_w, (void *)&thread_id[1]);


	pthread_attr_init(&attr3); // 2.
	pthread_attr_setinheritsched(&attr3, PTHREAD_EXPLICIT_SCHED); //Freigabe der Parameteraenderung
	pthread_attr_setschedpolicy(&attr3, SCHED_FIFO);
	my_prio3.sched_priority = 10; // 4.
	pthread_attr_setschedparam(&attr3, &my_prio3);
	pthread_create(&threads[2], &attr3, p_2_w, (void *) &thread_id[2]);


	printf("1: %d \n", pthread_attr_init(&attr4)); // 2.
	printf("2: %d \n", pthread_attr_setinheritsched(&attr4, PTHREAD_EXPLICIT_SCHED)); //Freigabe der Parameteraenderung
	printf("3: %d \n", pthread_attr_setschedpolicy(&attr4, SCHED_FIFO));
	my_prio4.sched_priority = 10; // 4.
	printf("4: %d \n", pthread_attr_setschedparam(&attr4, &my_prio4));
	printf("5: %d \n", pthread_create(&threads[3], &attr4, consumer, (void *) &thread_id[3]));



//	pthread_create(&threads[0], NULL, control, (void *)&thread_id[0]);
//	pthread_create(&threads[1], NULL, p_1_w, (void *)&thread_id[1]);
//	pthread_create(&threads[2], NULL, p_2_w, (void *) &thread_id[2]);
//	pthread_create(&threads[3], NULL, consumer, (void *) &thread_id[3]);
	
	for(i = 0; i < 4; i++)
		pthread_join(threads[i], thread_result[i]);

	printf("Ende nach Join der Threads\n");
	return 0;
}
