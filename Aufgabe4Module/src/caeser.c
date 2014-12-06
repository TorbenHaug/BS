/*
 * caeser.c
 *
 *  Created on: 06.12.2014
 *      Author: torbenhaug
 */
#include "caeser.h"

/**
 * table for shifting
 */
const char shift_table[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz"};

int find_index(const char a[],const int size, const char value)
{
   int i = 0;
   for (i=0; i<size; i++)
   {
	 if (a[i] == value)
	 {
	    return i;
	 }
   }
   return(-1);
}

char caeser(char move_char, int shift){
	int index = find_index(shift_table, NELEMS(shift_table), move_char);
	if(index >= 0){

		int new_index = (index + shift);
		while (new_index<0){
			new_index = new_index + NELEMS(shift_table);
		}
		new_index %= NELEMS(shift_table);
		//printf("%d:%d:%d",index,new_index,shift);
		move_char = shift_table[new_index];
	}
	return move_char;
}
/*
int main(){
	int i = 0;
	for(i = 0; i < NELEMS(shift_table);i++){
		printf("%c", caeser(shift_table[i],0));
	}
	printf("\n");
	for(i = 0; i < NELEMS(shift_table);i++){
		printf("%c", caeser(shift_table[i],1));
	}
	printf("\n");
	for(i = 0; i < NELEMS(shift_table);i++){
		printf("%c", caeser(shift_table[i],(-1)));
	}
	printf("\n");
	for(i = 0; i < NELEMS(shift_table);i++){
		printf("%c", caeser(shift_table[i],(4000)));
	}
	return 0;
}*/

