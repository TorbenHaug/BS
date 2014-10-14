void* write_c(void *pid)
{
	int i = 0;
	int z_var = 0;
	// kein static erforderlich, da jeder Thread seinen eigenen Kontext hat
	printf("Start Schreiben; %d: \n", *(int*)pid);
	while(1)
	{
		i++;
		z_var++;
		if (z_var > 9) z_var = 0;
		pthread_mutex_lock(&rb_mutex);
		while(p_rb -> p_in == p_rb -> p_out && p_rb ->count ==MAX)
		{
			printf("Full: wait\n");
			pthread_cond_wait(&not_full_condvar, &rb_mutex);
			printf("Aufgewacht: count %d, Thread_Nr. %d\n",
											p_rb -> count,
											*(int*)pid);
		}
		*(p_rb -> p_in) = z_var;
		(p_rb -> p_in)++ ;
		if((p_rb -> p_in) > p_end)
		{
			p_rb -> p_in = p_start;
		}
		(p_rb -> count)++;
		if(p_rb -> count != 0)
		{
			printf("Buffer nicht leer, signalisiert\n");
			pthread_cond_signal(&not_empty_condvar);
		}
		pthread_mutex_unlock(&rb_mutex);
		sleep(1);
	}
	return (NULL);
}