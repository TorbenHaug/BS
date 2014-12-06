/**
 * Translate (Caeser encryption) Module for Linuxkernel.
 * There will be added two devices to your dev folder: /dev/trans0, /dev/trans1
 * trans0 will encrypt a given chararray and trans0 will decrypt it.
 */


/**
 * Char de-/encrypter.
 * This function will move one char on the filetable by a given number.
 * The chars have to be in the following range: ['A'-'Z'] | [' '] | ['a'-'z'].
 * All other chars will only reached througt this function.
 * @move_char - char to be de-/encrypted
 * @shift	  - number of letters wich the char will be move, + means right, - means left
 * 				caeser('b',1) -> 'c', caeser('b',-1) -> 'a', caeser('ß',-1) -> 'ß'
 */
static char caeser(char move_char, int shift){

}
