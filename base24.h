#ifndef BASE24_H_INCLUDED
#define BASE24_H_INCLUDED

/*
 * base24.h
 * 
 * Provides base-24 conversion functions and definitions for use in
 * NELSC.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/*
 * The minimum integer value that a signed base-24 pair can represent.
 */
#define BASE24_PAIR_MIN (-96)

/*
 * The maximum integer value that a signed base-24 pair can represent.
 */
#define BASE24_PAIR_MAX (479)

/*
 * The maximum integer value that an unsigned base-24 digit can
 * represent.
 */
#define BASE24_DIGIT_MAX (23)

/*
 * Convert a signed base-24 pair into a signed integer.
 * 
 * str must point to the first base-24 character of a base-24 pair.  If
 * *str is the null character, the function will fail without trying to
 * read the next character, so it is safe to use this function within
 * null terminated strings without worrying about it reading past the
 * end of the string.  This function does not skip over whitespace, so
 * it will fail if *str is a whitespace character.
 * 
 * If the two characters at *str can't be parsed as a base-24 pair, the
 * function will fail, returning false.  Otherwise, it will return true
 * and write the converted result to *pResult.
 * 
 * The alphabetic base-24 digits are case insensitive; either upper or
 * lowercase will work.
 * 
 * The converted result will always be in the range BASE24_PAIR_MIN up
 * to BASE24_PAIR_MAX (inclusive of boundaries).
 * 
 * Parameters:
 * 
 *   str - pointer to the base-24 digit pair to convert
 * 
 *   pResult - pointer to the variable to receive the converted result
 *   if conversion is successful
 * 
 * Return:
 * 
 *   true if the base-24 was successfully parsed, false if parsing
 *   failed
 * 
 * Faults:
 * 
 *   - If str is NULL
 * 
 *   - If pResult is NULL
 * 
 * Undefined behavior:
 * 
 *   - If str is not null-terminated
 */
bool base24_pairToInt(const char *str, int32_t *pResult);

/*
 * Print the given signed integer value as a base-24 pair in ASCII,
 * writing the output to the given file.
 * 
 * The integer value must be in range BASE24_PAIR_MIN to BASE24_PAIR_MAX
 * (inclusive of boundaries) or a fault will occur.
 * 
 * Alphabetic base-24 characters are always written in uppercase.
 * 
 * Parameters:
 * 
 *   pFile - the file to write to
 * 
 *   v - the signed value to write as a base-24 pair
 * 
 * Faults:
 * 
 *   - If pFile is NULL
 * 
 *   - If v is out of range
 * 
 *   - If writing the characters to the file fails
 */
void base24_printPair(FILE *pFile, int32_t v);

/*
 * Convert the given base-24 digit into its unsigned integer value and
 * return it.
 * 
 * If the provided character is not an ASCII base-24 digit, the function
 * fails and -1 is returned.
 * 
 * The alphabetic base-24 digits are case insensitive; either upper or
 * lowercase will work.
 * 
 * The converted result will always be in the range zero up to
 * BASE24_DIGIT_MAX (inclusive of boundaries).
 * 
 * Parameters:
 * 
 *   c - the base-24 digit to convert
 * 
 * Return:
 * 
 *   the unsigned integer value of the base-24 digit, or -1 if the
 *   character is not a base-24 digit
 */
int32_t base24_digitToInt(char c);

/*
 * Convert a given unsigned integer value to an ASCII base-24 digit.
 * 
 * The integer value must be in range 0 to BASE24_DIGIT_MAX (inclusive
 * of boundaries) or a fault will occur.
 * 
 * Alphabetic base-24 digits are always returned in uppercase.
 * 
 * Parameters:
 * 
 *   v - the unsigned integer value to convert
 * 
 * Return:
 * 
 *   the base-24 digit
 * 
 * Faults:
 * 
 *   - If v is out of range
 */
char base24_intToDigit(int32_t v);

#endif
