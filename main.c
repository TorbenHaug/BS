#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#define MAX 16
pthread_mutex_t rb_mutex = PTHREAD_MUTEX_INITIALIZER;

// TODO

pthread_cond_t not_empty_condvar = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full_condvar = PTHREAD_COND_INITIALIZER;
pthread_cond_t prod_1_restart = PTHREAD_COND_INITIALIZER;
pthread_cond_t prod_2_restart = PTHREAD_COND_INITIALIZER;
pthread_cond_t cons_restart = PTHREAD_COND_INITIALIZER;

int prod_1_stopped = 0;
int prod_2_stopped = 0;
int cons_stopped = 0;

int thread_id[4] = {0,1,2,3};
typedef struct {
	int buffer[MAX];
	int *p_in;
	int *p_out;
	int count;
}rb;
rb x = { {0}, NULL, NULL, 0};
rb *p_rb = &x;

#define p_start (int *)(p_rb -> buffer)
#define p_end (int *)((p_rb -> buffer) + MAX-1)

void* p_1_w(void *pid);
void* p_2_w(void *pid);
void* consumer(void *pid);
void* control(void *pid);

int main(int argc, char* argv[])
{
	int i;
	pthread_t threads[4];
	printf("Start des Beispiels \n");
	//printf("Argumente verfuegbar: ARGC\n", 3*argc);
	
	// TODO
	
	p_rb -> p_in = p_start;
	p_rb -> p_out = p_start;
	p_rb -> count = 0;
	printf("Counter value %d\n", p_rb ->count);
	pthread_create(&threads[0], NULL, …, (void *)thread_id);
	pthread_create(&threads[1], NULL, …, (void *)&thread_id[1]);
	pthread_create(&threads[2], NULL, …, (void *) &thread_id[2]);
	
	// TODO

	for(i = 0; i<4; i++)
		pthread_join(threads[i], NULL);
		
	printf("Ende nach Join der Threads\n");
	return 0;
}