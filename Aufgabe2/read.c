void* read_rb(void *pid)
{
	int i = 0;
	printf("Start Lesen; %d: \n", *(int*)pid);
	while(1)
	{
		i++;
		pthread_mutex_lock(&rb_mutex);
		while(p_rb -> count == 0 || !stoped)
		{
			printf("Empty: wait\n");
			pthread_cond_wait(&not_empty_condvar, &rb_mutex);
			printf("Aufgewacht: count %d, Thread_Nr. %d\n",
												p_rb -> count,
												*(int*)pid);
		
			pthread_cond_wait(&not_stoped_condvar, &rb_mutex);
		}
		(p_rb -> count)--;
		printf("Ältestes Zeichen ausgeben: %d:\n", *(p_rb -> p_out));
		(p_rb -> p_out)++;
		if((p_rb -> p_out) > p_end)
		{
			p_rb -> p_out = p_start;
		}
		if(p_rb -> count <= MAX)
		{
			printf("Buffer nicht voll, signalisiert\n");
			pthread_cond_signal(&not_full_condvar);
		}
		pthread_mutex_unlock(&rb_mutex);
		sleep(1);
	}
	return (NULL);
}