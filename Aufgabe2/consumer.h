/*
 * consumer.h
 *
 *  Created on: 14.10.2014
 *      Author: torbenhaug
 * Include this file for using consumer
 */

#ifndef CONSUMER_H_
#define CONSUMER_H_
	#include <pthread.h>
	#include <stdio.h>

	void* consumer(void *pid);
	#ifndef MAX
		#define MAX 16
	#endif
	#ifndef p_start
		#define p_start (int *)(p_rb -> buffer)
	#endif
	#ifndef p_end
		#define p_end (int *)((p_rb -> buffer) + MAX-1)
	#endif

#endif /* CONSUMER_H_ */
