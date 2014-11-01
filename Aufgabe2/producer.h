/*
 * producer.h
 *
 *  Created on: 29.10.2014
 *      Author: torbenhaug
 */

#ifndef PRODUCER_H_
#define PRODUCER_H_
	void* p_1_w(void *pid);
	void* p_2_w(void *pid);
	#ifndef MAX
		#define MAX 16
	#endif
	#ifndef p_start
		#define p_start (int *)(p_rb -> buffer)
	#endif
	#ifndef p_end
		#define p_end (int *)((p_rb -> buffer) + MAX-1)
	#endif

#endif /* PRODUCER_H_ */
