/*
 * caeser.h
 *
 *  Created on: 06.12.2014
 *      Author: torbenhaug
 */

#ifndef SRC_CAESER_H_
#define SRC_CAESER_H_
	#define NELEMS(x)  ((sizeof(x) / sizeof(x[0])-1))
	/**
	 * Char de-/encrypter.
	 * This function will move one char on the filetable by a given number.
	 * The chars have to be in the following range: ['A'-'Z'] | [' '] | ['a'-'z'].
	 * All other chars will only reached througt this function.
	 * @move_char - char to be de-/encrypted
	 * @shift	  - number of letters wich the char will be move, + means right, - means left
	 * 				caeser('b',1) -> 'c', caeser('b',-1) -> 'a', caeser('ß',-1) -> 'ß'
	 */
	char caeser(char move_char, int shift);


#endif /* SRC_CAESER_H_ */
