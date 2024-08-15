/** basic character functions
 * @since 0.0.1
 * @secreflist
 * Dependencies:
 * - stdint.h
 * - stdbool.h
 * @endsecreflist
*/
#include "ctype.h"

/** check is lowercase
 * @param chr		Character to check
 * @return bool		True if the character is lowercase, false otherwise.
 * @since 0.0.1
 * @headerfile ctype.h
 * Check if char value is beetween 'a' and 'z'
*/
bool islower(char chr) {
	return chr >= 'a' && chr <= 'z';
}

/** convert to uppercase
 * @param chr		Character to convert
 * @return char		Converted character
 * @since 0.0.1
 * @headerfile ctype.h
 * Add char value of 'A' - 'a' if chr is lowercase
*/
char toupper(char chr) {
	return islower(chr) ? (chr - 'a' + 'A') : chr;
}